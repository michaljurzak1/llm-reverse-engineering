import os
import typer
from typing import Optional
from pathlib import Path
from dotenv import load_dotenv
from loguru import logger
from rich.console import Console
from rich.panel import Panel
from rich.table import Table
import json

from tools.radare2_tools import Radare2Tools
from agents.reverse_engineering_agent import ReverseEngineeringAgent
from tools.code_generator import CodeGenerator

# Initialize Typer app
app = typer.Typer(help="LLM-powered Reverse Engineering Tool")
console = Console()

# Load environment variables
load_dotenv()

def setup_logging():
    """Configure logging settings."""
    log_file = os.getenv("LOG_FILE", "logs/analysis.log")
    log_level = os.getenv("LOG_LEVEL", "INFO")
    
    # Create logs directory if it doesn't exist
    Path("logs").mkdir(exist_ok=True)
    
    logger.add(
        log_file,
        level=log_level,
        rotation="1 day",
        retention="7 days",
        format="{time:YYYY-MM-DD HH:mm:ss} | {level} | {message}"
    )

def save_output(output: str, output_file: Optional[Path] = None):
    """Save the analysis output to a file."""
    if output_file:
        try:
            output_file.parent.mkdir(parents=True, exist_ok=True)
            with open(output_file, 'w') as f:
                f.write(output)
            logger.info(f"Analysis output saved to {output_file}")
        except Exception as e:
            logger.error(f"Failed to save output to {output_file}: {e}")
            raise

@app.command()
def generate(
    filepath: str = typer.Argument(..., help="Path to the binary file to analyze"),
    mode: str = typer.Option(
        os.getenv("DEFAULT_ANALYSIS_MODE", "standard"),
        "--mode",
        "-m",
        help="Analysis mode (quick/standard/deep)"
    ),
    llm_type: str = typer.Option(
        "local",
        "--llm-type",
        "-t",
        help="Type of LLM to use (local/openai)"
    ),
    output: Optional[Path] = typer.Option(
        None,
        "--output",
        "-o",
        help="Path to save the analysis output",
        file_okay=True,
        dir_okay=False,
        writable=True,
        resolve_path=True
    )
):
    """Analyze a binary file using LLM-powered reverse engineering."""
    try:
        # Setup logging
        setup_logging()
        
        # Initialize components
        console.print(Panel.fit("Initializing Reverse Engineering Analysis"))
        
        # Initialize Radare2 tools
        r2_tools = Radare2Tools(filepath, mode)
        
        # Initialize the agent
        agent = ReverseEngineeringAgent(llm_type, mode)
        
        # Create the agent executor with the tools
        agent_executor = agent.create_agent(r2_tools.get_tools())
        
        # Start the analysis
        console.print(Panel.fit("Starting Analysis"))
        
        # Initial analysis query
        query = f"Please analyze the binary file at {filepath} in {mode} mode."
        result = agent_executor.invoke({
            "input": query,
            "history": agent.history
        })
        
        # Update history
        agent.update_history(query, result)
        
        # Display results
        console.print(Panel.fit("Analysis Results", style="bold green"))
        console.print(result["output"])
        
        # Save output if specified
        if output:
            save_output(result["output"], output)
        
        # Cleanup
        r2_tools.r2.quit()
        
    except Exception as e:
        logger.error(f"Analysis failed: {e}")
        console.print(f"[red]Error: {str(e)}[/red]")
        raise typer.Exit(1)

@app.command()
def generate_and_analyze(
    n: int = typer.Option(
        1,
        "--number",
        "-n",
        help="Number of C programs to generate"
    ),
    output_dir: str = typer.Option(
        "generated_codes",
        "--output-dir",
        "-d",
        help="Directory to store generated codes"
    ),
    mode: str = typer.Option(
        "standard",
        "--mode",
        "-m",
        help="Analysis mode (quick/standard/deep)"
    ),
    llm_type: str = typer.Option(
        "local",
        "--llm-type",
        "-t",
        help="Type of LLM to use (local/openai)"
    )
):
    """Generate C programs using CSmith and analyze them."""
    try:
        # Setup logging
        setup_logging()
        
        # Initialize components
        console.print(Panel.fit("Initializing Code Generation and Analysis"))
        
        # Initialize code generator
        code_gen = CodeGenerator(output_dir)
        
        # Generate C programs
        console.print(Panel.fit("Generating C Programs"))
        c_files = code_gen.generate_c_code(n)
        
        results_table = Table(title="Analysis Results")
        results_table.add_column("Original File")
        results_table.add_column("Decompiled File")
        results_table.add_column("Size Difference")
        results_table.add_column("MD5 Match")
        results_table.add_column("CPG Similarity")
        results_table.add_column("Node Difference")
        results_table.add_column("Edge Difference")
        
        for c_file in c_files:
            try:
                # Compile original code
                original_binary = code_gen.compile_c_code(c_file)
                
                # Initialize Radare2 tools with the binary file
                r2_tools = Radare2Tools(str(original_binary), mode)
                agent = ReverseEngineeringAgent(llm_type, mode)
                agent_executor = agent.create_agent(r2_tools.get_tools())
                
                # Analyze with Radare2
                query = f"Please analyze the binary file at {original_binary} in {mode} mode."
                result = agent_executor.invoke({
                    "input": query,
                    "history": agent.history
                })
                
                # Extract and save decompiled code
                decompiled_code = code_gen.extract_c_code_from_markdown(result["output"])
                decompiled_file = code_gen.save_decompiled_code(decompiled_code, c_file)
                
                # Generate CPGs for both files
                original_cpg = code_gen.generate_cpg(c_file)
                decompiled_cpg = code_gen.generate_cpg(decompiled_file)
                
                # Compare CPGs
                cpg_comparison = code_gen.compare_cpgs(original_cpg, decompiled_cpg)
                
                # Compile decompiled code
                decompiled_binary = code_gen.compile_c_code(decompiled_file)
                
                # Compare binaries
                comparison = code_gen.compare_binaries(original_binary, decompiled_binary)
                
                # Add results to table
                results_table.add_row(
                    str(c_file),
                    str(decompiled_file),
                    f"{comparison['file_size']['percentage']:.2f}%",
                    "Yes" if comparison['md5_hash']['match'] else "No",
                    f"{cpg_comparison['similarity_score']:.2%}",
                    str(cpg_comparison['node_difference']),
                    str(cpg_comparison['edge_difference'])
                )
                
                # Save detailed CPG analysis
                analysis_file = c_file.with_name(f"{c_file.stem}_cpg_analysis.json")
                with open(analysis_file, 'w') as f:
                    json.dump({
                        'cpg_comparison': cpg_comparison,
                        'binary_comparison': comparison
                    }, f, indent=2)
                
                # Cleanup Radare2 instance for this file
                r2_tools.r2.quit()
                
            except Exception as e:
                logger.error(f"Failed to analyze {c_file}: {e}")
                results_table.add_row(
                    str(c_file),
                    "Failed",
                    "N/A",
                    "N/A",
                    "N/A",
                    "N/A",
                    "N/A"
                )
        
        # Display results
        console.print(Panel.fit("Analysis Results"))
        console.print(results_table)
        
    except Exception as e:
        logger.error(f"Analysis failed: {e}")
        console.print(f"[red]Error: {str(e)}[/red]")
        raise typer.Exit(1)

if __name__ == "__main__":
    app()
