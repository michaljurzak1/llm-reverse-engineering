import os
import subprocess
from pathlib import Path
from typing import List, Dict, Any
from loguru import logger
import re
import hashlib
import json
import networkx as nx
from difflib import SequenceMatcher
import tempfile

class CodeGenerator:
    def __init__(self, output_dir: str = "generated_codes"):
        self.output_dir = Path(output_dir)
        self.output_dir.mkdir(parents=True, exist_ok=True)
        self.csmith_path = "csmith"  # Assuming csmith is in PATH
        self.joern_path = "joern-parse"  # Assuming joern is in PATH
        self.joern_export = "joern-export"  # Assuming joern-export is in PATH

    def generate_c_code(self, n: int = 1) -> List[Path]:
        """Generate n C programs using CSmith."""
        generated_files = []
        for i in range(n):
            output_file = self.output_dir / f"test_{i}.c"
            try:
                # Generate C code using CSmith
                subprocess.run([
                    self.csmith_path,
                    "--output", str(output_file)
                ], check=True)
                generated_files.append(output_file)
                logger.info(f"Generated C code: {output_file}")
            except subprocess.CalledProcessError as e:
                logger.error(f"Failed to generate C code: {e}")
                raise
        return generated_files

    def compile_c_code(self, c_file: Path) -> Path:
        """Compile C code without optimizations."""
        binary_file = c_file.with_suffix('')
        try:
            subprocess.run([
                "gcc",
                "-O0",  # No optimizations
                "-I/usr/include/csmith",  # Include CSmith headers
                str(c_file),
                "-o", str(binary_file)
            ], check=True)
            logger.info(f"Compiled binary: {binary_file}")
            return binary_file
        except subprocess.CalledProcessError as e:
            logger.error(f"Failed to compile {c_file}: {e}")
            raise

    def extract_c_code_from_markdown(self, markdown: str) -> str:
        """Extract C code from markdown output."""
        # Look for code blocks with C language specification
        pattern = r"```c\n(.*?)```"
        matches = re.findall(pattern, markdown, re.DOTALL)
        if not matches:
            raise ValueError("No C code block found in markdown")
        return matches[0].strip()

    def save_decompiled_code(self, code: str, original_file: Path) -> Path:
        """Save decompiled code to a file."""
        decompiled_file = original_file.with_name(f"{original_file.stem}_decompiled.c")
        with open(decompiled_file, 'w') as f:
            f.write(code)
        return decompiled_file

    def compare_binaries(self, original_binary: Path, decompiled_binary: Path) -> Dict[str, Any]:
        """Compare two binaries using various methods."""
        results = {
            "file_size": self._compare_file_sizes(original_binary, decompiled_binary),
            "md5_hash": self._compare_md5_hashes(original_binary, decompiled_binary),
            "strings": self._compare_strings(original_binary, decompiled_binary)
        }
        return results

    def _compare_file_sizes(self, file1: Path, file2: Path) -> Dict[str, Any]:
        """Compare file sizes of two binaries."""
        size1 = file1.stat().st_size
        size2 = file2.stat().st_size
        return {
            "file1_size": size1,
            "file2_size": size2,
            "difference": abs(size1 - size2),
            "percentage": abs(size1 - size2) / max(size1, size2) * 100
        }

    def _compare_md5_hashes(self, file1: Path, file2: Path) -> Dict[str, Any]:
        """Compare MD5 hashes of two binaries."""
        def get_md5(file_path: Path) -> str:
            with open(file_path, 'rb') as f:
                return hashlib.md5(f.read()).hexdigest()
        
        hash1 = get_md5(file1)
        hash2 = get_md5(file2)
        return {
            "file1_hash": hash1,
            "file2_hash": hash2,
            "match": hash1 == hash2
        }

    def _compare_strings(self, file1: Path, file2: Path) -> Dict[str, Any]:
        """Compare strings in two binaries."""
        def get_strings(file_path: Path) -> List[str]:
            result = subprocess.run(
                ["strings", str(file_path)],
                capture_output=True,
                text=True
            )
            return result.stdout.splitlines()
        
        strings1 = set(get_strings(file1))
        strings2 = set(get_strings(file2))
        
        common = strings1.intersection(strings2)
        unique1 = strings1 - strings2
        unique2 = strings2 - strings1
        
        return {
            "common_strings": len(common),
            "unique_to_file1": len(unique1),
            "unique_to_file2": len(unique2),
            "total_strings_file1": len(strings1),
            "total_strings_file2": len(strings2)
        }

    def _export_to_dot(self, cpg_bin: Path, output_dir: Path) -> Path:
        """Export CPG to dot format using joern-export."""
        try:
            subprocess.run(
                [
                    self.joern_export,
                    "--repr", "all",
                    "--format", "dot",
                    str(cpg_bin),
                    "-o", str(output_dir)
                ],
                check=True,
                capture_output=True,
                text=True
            )
            dot_file = output_dir / "export.dot"
            if not dot_file.exists():
                raise RuntimeError(f"Export dot file not found at {dot_file}")
            return dot_file
        except subprocess.CalledProcessError as e:
            logger.error(f"Failed to export CPG to dot: {e}")
            raise

    def generate_cpg(self, c_file: Path) -> Dict[str, Any]:
        """Generate Code Property Graph using Joern."""
        cpg_bin = c_file.with_suffix('.cpg.bin')
        try:
            # Generate CPG using Joern
            logger.info(f"Generating CPG for {c_file}")
            joern_result = subprocess.run(
                [
                    self.joern_path,
                    "--output", str(cpg_bin),
                    "--language", "c",
                    str(c_file)
                ],
                capture_output=True,
                text=True
            )
            
            if joern_result.returncode != 0:
                logger.error(f"Joern failed: {joern_result.stderr}")
                raise RuntimeError(f"Joern failed: {joern_result.stderr}")

            # Create temporary directory for dot export
            with tempfile.TemporaryDirectory() as temp_dir:
                dot_file = self._export_to_dot(cpg_bin, Path(temp_dir))
                # Read the dot file and convert to NetworkX graph
                G = nx.drawing.nx_agraph.read_dot(dot_file)
                
            return {
                'graph': G,
                'node_count': G.number_of_nodes(),
                'edge_count': G.number_of_edges()
            }
            
        except Exception as e:
            logger.error(f"Failed to generate CPG for {c_file}: {e}")
            return self._empty_cpg_result()
        finally:
            # Cleanup temporary files
            if cpg_bin.exists():
                cpg_bin.unlink()

    def _empty_cpg_result(self) -> Dict[str, Any]:
        """Return an empty CPG result structure."""
        return {
            'graph': nx.DiGraph(),
            'node_count': 0,
            'edge_count': 0
        }

    def compare_cpgs(self, cpg1: Dict[str, Any], cpg2: Dict[str, Any]) -> Dict[str, Any]:
        """Compare two Code Property Graphs using various similarity measures."""
        G1 = cpg1['graph']
        G2 = cpg2['graph']
        
        try:
            # Graph edit distance (with timeout to prevent long computations)
            try:
                ged = nx.graph_edit_distance(G1, G2, timeout=10)
            except (nx.NetworkXError, TimeoutError):
                ged = None
            
            # SimRank similarity for node pairs
            try:
                simrank = nx.simrank_similarity(G1)
                avg_simrank = sum(sum(row.values()) for row in simrank.values()) / (len(simrank) * len(simrank))
            except:
                avg_simrank = None
            
            # Basic structural similarity measures
            node_diff = abs(G1.number_of_nodes() - G2.number_of_nodes())
            edge_diff = abs(G1.number_of_edges() - G2.number_of_edges())
            density_diff = abs(nx.density(G1) - nx.density(G2))
            
            # Calculate overall similarity score
            similarity_score = 0.0
            score_components = 0
            
            if ged is not None:
                max_possible_ged = max(G1.number_of_nodes() + G1.number_of_edges(),
                                    G2.number_of_nodes() + G2.number_of_edges())
                similarity_score += 1 - (ged / max_possible_ged)
                score_components += 1
            
            if avg_simrank is not None:
                similarity_score += avg_simrank
                score_components += 1
            
            max_nodes = max(G1.number_of_nodes(), G2.number_of_nodes())
            if max_nodes > 0:
                similarity_score += 1 - (node_diff / max_nodes)
                score_components += 1
            
            max_edges = max(G1.number_of_edges(), G2.number_of_edges())
            if max_edges > 0:
                similarity_score += 1 - (edge_diff / max_edges)
                score_components += 1
            
            if score_components > 0:
                similarity_score /= score_components
            
            return {
                'similarity_score': similarity_score,
                'graph_edit_distance': ged,
                'avg_simrank_similarity': avg_simrank,
                'node_difference': node_diff,
                'edge_difference': edge_diff,
                'density_difference': density_diff
            }
            
        except Exception as e:
            logger.error(f"Error comparing CPGs: {e}")
            return {
                'similarity_score': 0.0,
                'graph_edit_distance': None,
                'avg_simrank_similarity': None,
                'node_difference': None,
                'edge_difference': None,
                'density_difference': None
            } 