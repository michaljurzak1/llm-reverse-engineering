import r2pipe
from typing import Dict, List, Optional, Any
from loguru import logger
import json
from langchain_core.tools import tool

class Radare2Tools:
    def __init__(self, filepath: str, analysis_mode: str = "standard"):
        self.filepath = filepath
        self.analysis_mode = analysis_mode
        self.r2 = None
        self._open_file()
        self.tools = self._create_tools()

    def _open_file(self) -> None:
        """Open the binary file with radare2."""
        try:
            self.r2 = r2pipe.open(self.filepath)
            logger.info(f"Successfully opened file: {self.filepath}")
        except Exception as e:
            logger.error(f"Failed to open file: {e}")
            raise

    def _create_tools(self) -> List[Any]:
        """Create LangChain tools from Radare2 methods."""
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
        def list_functions() -> List[Dict[str, Any]]:
            """List all functions in the binary."""
            try:
                functions = self.r2.cmdj("aflj")
                return functions
            except Exception as e:
                logger.error(f"Failed to list functions: {e}")
                return []

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

        @tool
        def search_strings() -> List[Dict[str, Any]]:
            """Search for strings in the binary."""
            try:
                strings = self.r2.cmdj("izj")
                return strings
            except Exception as e:
                logger.error(f"Failed to search strings: {e}")
                return []

        @tool
        def get_imports() -> List[Dict[str, Any]]:
            """Get all imports from the binary."""
            try:
                imports = self.r2.cmdj("iij")
                return imports
            except Exception as e:
                logger.error(f"Failed to get imports: {e}")
                return []

        @tool
        def get_exports() -> List[Dict[str, Any]]:
            """Get all exports from the binary."""
            try:
                exports = self.r2.cmdj("iEj")
                return exports
            except Exception as e:
                logger.error(f"Failed to get exports: {e}")
                return []

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

        @tool
        def close() -> None:
            """Close the radare2 session."""
            if self.r2:
                self.r2.quit()
                logger.info("Closed radare2 session")

        return [
            analyze,
            list_functions,
            decompile,
            search_strings,
            get_imports,
            get_exports,
            read_memory,
            close
        ]

    def get_tools(self) -> List[Any]:
        """Get the list of available tools."""
        return self.tools 