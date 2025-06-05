import r2pipe
from typing import Dict, List, Optional, Any, Union
from loguru import logger
import json
from langchain_core.tools import tool
from pathlib import Path
import subprocess
import os

class Radare2Tools:
    def __init__(self, filepath: str, analysis_mode: str = "standard", is_project_dir: bool = False):
        self.filepath = filepath
        self.analysis_mode = analysis_mode
        self.is_project_dir = is_project_dir
        self.r2 = None
        self.project_files = {}
        self._initialize_analysis()
        self.tools = self._create_tools()

    def _initialize_analysis(self) -> None:
        """Initialize analysis based on whether it's a single file or project directory."""
        if self.is_project_dir:
            self._initialize_project_analysis()
        else:
            self._open_file()

    def _initialize_project_analysis(self) -> None:
        """Initialize analysis for a project directory."""
        try:
            project_path = Path(self.filepath)
            if not project_path.is_dir():
                raise ValueError(f"Expected directory path, got file: {self.filepath}")

            # Find all binary files and object files
            for file_path in project_path.rglob("*"):
                if file_path.is_file() and not file_path.name.startswith('.'):
                    # Check if it's a binary or object file
                    try:
                        file_type = subprocess.check_output(['file', str(file_path)], text=True)
                        if any(x in file_type.lower() for x in ['elf', 'executable', 'object', 'shared object']):
                            self.project_files[file_path.name] = str(file_path)
                    except subprocess.CalledProcessError:
                        continue

            if not self.project_files:
                raise ValueError(f"No binary files found in project directory: {self.filepath}")

            # Open the main binary if it exists, otherwise open the first binary found
            main_binary = next((path for name, path in self.project_files.items() 
                              if name in ['main', 'app', 'program']), 
                             next(iter(self.project_files.values())))
            self.r2 = r2pipe.open(main_binary)
            logger.info(f"Successfully opened project directory: {self.filepath}")
            logger.info(f"Found {len(self.project_files)} binary files")
        except Exception as e:
            logger.error(f"Failed to initialize project analysis: {e}")
            raise

    def _open_file(self) -> None:
        """Open a single binary file with radare2."""
        try:
            self.r2 = r2pipe.open(self.filepath)
            logger.info(f"Successfully opened file: {self.filepath}")
        except Exception as e:
            logger.error(f"Failed to open file: {e}")
            raise

    def search_strings(self) -> List[str]:
        """Search for strings in the binary and return them as a list."""
        try:
            strings = self.r2.cmdj("izj")
            return [s.get('string', '') for s in strings if 'string' in s]
        except Exception as e:
            logger.error(f"Failed to search strings: {e}")
            return []

    def get_imports(self) -> List[str]:
        """Get all imports from the binary as a list of strings."""
        try:
            imports = self.r2.cmdj("iij")
            return [imp.get('name', '') for imp in imports if 'name' in imp]
        except Exception as e:
            logger.error(f"Failed to get imports: {e}")
            return []

    def get_exports(self) -> List[str]:
        """Get all exports from the binary as a list of strings."""
        try:
            exports = self.r2.cmdj("iEj")
            return [exp.get('name', '') for exp in exports if 'name' in exp]
        except Exception as e:
            logger.error(f"Failed to get exports: {e}")
            return []

    def get_call_graph(self) -> Dict[str, List[str]]:
        """Get the call graph of the binary."""
        try:
            # Get all functions
            functions = self.r2.cmdj("aflj")
            call_graph = {}
            
            for func in functions:
                if 'name' in func:
                    # Get function references
                    refs = self.r2.cmdj(f"axtj @ {func['name']}")
                    call_graph[func['name']] = [
                        ref.get('refname', '') for ref in refs 
                        if 'refname' in ref
                    ]
            
            return call_graph
        except Exception as e:
            logger.error(f"Failed to get call graph: {e}")
            return {}

    def get_binary_info(self) -> Dict[str, Any]:
        """Get general information about the binary."""
        try:
            info = self.r2.cmdj("ij")
            return info
        except Exception as e:
            logger.error(f"Failed to get binary info: {e}")
            return {}

    def _create_tools(self) -> List[Any]:
        """Create LangChain tools from Radare2 methods."""
        tools = []

        @tool
        def analyze() -> Dict[str, Any]:
            """Perform analysis based on the selected mode."""
            try:
                if self.analysis_mode == "quick":
                    self.r2.cmd("aaa")
                elif self.analysis_mode == "standard":
                    self.r2.cmd("aaaa")
                else:  # deep
                    self.r2.cmd("aaaaa")
                return {"status": "success", "message": f"Analysis completed in {self.analysis_mode} mode"}
            except Exception as e:
                logger.error(f"Analysis failed: {e}")
                return {"status": "error", "message": str(e)}

        @tool
        def list_project_files() -> Dict[str, Any]:
            """List all binary files in the project directory."""
            if not self.is_project_dir:
                return {"status": "error", "message": "Not a project directory"}
            return {
                "status": "success",
                "files": self.project_files
            }

        @tool
        def analyze_project_file(filename: str) -> Dict[str, Any]:
            """Analyze a specific file from the project directory."""
            if not self.is_project_dir:
                return {"status": "error", "message": "Not a project directory"}
            if filename not in self.project_files:
                return {"status": "error", "message": f"File {filename} not found in project"}
            
            try:
                file_r2 = r2pipe.open(self.project_files[filename])
                file_r2.cmd("aaaa")
                info = file_r2.cmdj("ij")
                file_r2.quit()
                return {
                    "status": "success",
                    "file": filename,
                    "info": info
                }
            except Exception as e:
                logger.error(f"Failed to analyze project file {filename}: {e}")
                return {"status": "error", "message": str(e)}

        # Add existing tools
        tools.extend([
            analyze,
            self._create_list_functions_tool(),
            self._create_decompile_tool(),
            self._create_search_strings_tool(),
            self._create_get_imports_tool(),
            self._create_get_exports_tool(),
            self._create_read_memory_tool(),
            self._create_close_tool()
        ])

        # Add project-specific tools if in project mode
        if self.is_project_dir:
            tools.extend([
                list_project_files,
                analyze_project_file
            ])

        return tools

    def _create_list_functions_tool(self):
        @tool
        def list_functions(
            filter_type: str = "all",
            max_functions: int = 50,
            search_term: str = None,
            show_unused: bool = False
        ) -> Dict[str, Any]:
            """List functions in the binary with filtering options.
            
            Args:
                filter_type: Type of functions to list ('all', 'main', 'imported', 'exported', 'user')
                max_functions: Maximum number of functions to return.
                search_term: Optional search term to filter function names
                show_unused: If False, only show functions that are actually used in the program
            """
            try:
                # First, get all functions
                if filter_type == "main":
                    functions = self.r2.cmdj("aflj~main")
                elif filter_type == "imported":
                    functions = self.r2.cmdj("aflj~imp")
                elif filter_type == "exported":
                    functions = self.r2.cmdj("aflj~exp")
                elif filter_type == "user":
                    functions = self.r2.cmdj("aflj~sym")
                else:  # all
                    functions = self.r2.cmdj("aflj")

                if not functions:
                    return {"status": "error", "message": "No functions found"}

                # If we don't want to show unused functions, filter them out
                if not show_unused:
                    used_functions = set()
                    
                    # Get all function references
                    for func in functions:
                        func_name = func.get('name', '')
                        if func_name:
                            # Get references to this function
                            refs = self.r2.cmdj(f"axtj @ {func_name}")
                            if refs:
                                used_functions.add(func_name)
                            
                            # Get calls from this function
                            calls = self.r2.cmdj(f"axfj @ {func_name}")
                            if calls:
                                for call in calls:
                                    if 'fcn_name' in call:
                                        used_functions.add(call['fcn_name'])
                    
                    # Filter functions to only include used ones
                    functions = [f for f in functions if f.get('name', '') in used_functions]

                # Filter by search term if provided
                if search_term:
                    functions = [
                        f for f in functions 
                        if search_term.lower() in f.get('name', '').lower()
                    ]

                # Sort functions by size (largest first) and limit the number
                functions.sort(key=lambda x: x.get('size', 0), reverse=True)
                functions = functions[:max_functions]

                # Create a more concise representation
                result = {
                    "status": "success",
                    "total_functions": len(functions),
                    "show_unused": show_unused,
                    "functions": [
                        {
                            "name": f.get('name', ''),
                            "size": f.get('size', 0),
                            "type": f.get('type', ''),
                            "address": f.get('offset', 0),
                            "is_used": f.get('name', '') in used_functions if not show_unused else True
                        }
                        for f in functions
                    ]
                }

                return result
            except Exception as e:
                logger.error(f"Failed to list functions: {e}")
                return {"status": "error", "message": str(e)}
        return list_functions

    def _create_decompile_tool(self):
        @tool
        def decompile(function_name: str) -> Dict[str, Any]:
            """Decompile a specific function."""
            try:
                decompiled = self.r2.cmd(f"pdc @ {function_name}")
                return {
                    "function": function_name,
                    "decompiled_code": decompiled
                }
            except Exception as e:
                logger.error(f"Failed to decompile function {function_name}: {e}")
                return {"function": function_name, "error": str(e)}
        return decompile

    def _create_search_strings_tool(self):
        @tool
        def search_strings() -> List[Dict[str, Any]]:
            """Search for strings in the binary."""
            try:
                strings = self.r2.cmdj("izj")
                return strings
            except Exception as e:
                logger.error(f"Failed to search strings: {e}")
                return []
        return search_strings

    def _create_get_imports_tool(self):
        @tool
        def get_imports() -> List[Dict[str, Any]]:
            """Get all imports from the binary."""
            try:
                imports = self.r2.cmdj("iij")
                return imports
            except Exception as e:
                logger.error(f"Failed to get imports: {e}")
                return []
        return get_imports

    def _create_get_exports_tool(self):
        @tool
        def get_exports() -> List[Dict[str, Any]]:
            """Get all exports from the binary."""
            try:
                exports = self.r2.cmdj("iEj")
                return exports
            except Exception as e:
                logger.error(f"Failed to get exports: {e}")
                return []
        return get_exports

    def _create_read_memory_tool(self):
        @tool
        def read_memory(address: str, size: int) -> Dict[str, Any]:
            """Read memory at a specific address."""
            try:
                data = self.r2.cmd(f"px {size} @ {address}")
                return {
                    "address": address,
                    "size": size,
                    "data": data
                }
            except Exception as e:
                logger.error(f"Failed to read memory at {address}: {e}")
                return {"address": address, "error": str(e)}
        return read_memory

    def _create_close_tool(self):
        @tool
        def close() -> None:
            """Close the radare2 session."""
            if self.r2:
                self.r2.quit()
                logger.info("Closed radare2 session")
        return close

    def get_tools(self) -> List[Any]:
        """Get the list of available tools."""
        return self.tools 