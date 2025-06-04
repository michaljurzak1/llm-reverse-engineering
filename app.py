import os
import streamlit as st
from pathlib import Path
from dotenv import load_dotenv
from loguru import logger
import json
from typing import Optional, List, Tuple # Adjusted Tuple import
import subprocess
from datetime import datetime
import networkx as nx
import matplotlib.pyplot as plt
from networkx.drawing.nx_agraph import graphviz_layout

from tools.radare2_tools import Radare2Tools
from agents.reverse_engineering_agent import ReverseEngineeringAgent
# from tools.code_generator import CodeGenerator # Not directly used, can be removed if not needed
from langchain_core.messages import HumanMessage, AIMessage # Crucial for llm_history

# Load environment variables
load_dotenv()

# Configure logging
def setup_logging():
    log_file = os.getenv("LOG_FILE", "logs/analysis.log")
    log_level = os.getenv("LOG_LEVEL", "INFO")
    Path("logs").mkdir(exist_ok=True)
    logger.add(
        log_file,
        level=log_level,
        rotation="1 day",
        retention="7 days",
        format="{time:YYYY-MM-DD HH:mm:ss} | {level} | {message}"
    )

def extract_c_code_from_markdown(markdown_text: str) -> str:
    code_blocks = []
    in_block = False
    current = []
    for line in markdown_text.splitlines():
        if line.strip().startswith("```c"):
            in_block = True
            current = [] # Reset current for new block
            continue
        if line.strip().startswith("```") and in_block:
            in_block = False
            if current: # Only append if there was content
                code_blocks.append("\n".join(current))
            current = []
            continue
        if in_block:
            current.append(line)
    # Handle unclosed block at the end of the text
    if in_block and current:
        code_blocks.append("\n".join(current))
    return "\n\n".join(code_blocks)


def compile_c_code(code: str, output_path_base: Path, optimize: bool = False) -> Tuple[Optional[Path], Optional[str]]:
    try:
        c_file = output_path_base.with_suffix(".c")
        with open(c_file, "w") as f:
            f.write(code)
        # output_path_base is now the stem for the binary, e.g., "decompiled_program"
        # The actual binary will be "decompiled_program" (no suffix on Linux/macOS)
        binary_executable_path = output_path_base
        opt_flag = "-O2" if optimize else "-O0"
        cmd = ["gcc", opt_flag, "-o", str(binary_executable_path.resolve()), str(c_file.resolve())]
        logger.info(f"Compile command: {' '.join(cmd)}")
        res = subprocess.run(cmd, capture_output=True, text=True, check=False)
        if res.returncode != 0:
            logger.error(f"GCC error: {res.stderr}")
            # Clean up .c file if compile fails to avoid confusion? Optional.
            # if c_file.exists(): c_file.unlink()
            return None, res.stderr
        logger.info(f"Compilation successful: {binary_executable_path.resolve()}")
        return binary_executable_path.resolve(), None
    except Exception as e:
        logger.error(f"Compile exception: {e}", exc_info=True)
        return None, str(e)

def generate_metrics(original: Path, decompiled: Path) -> dict:
    try:
        o_size, d_size = original.stat().st_size, decompiled.stat().st_size
        size_diff = abs(o_size - d_size)
        import hashlib
        def md5(p): return hashlib.md5(p.read_bytes()).hexdigest()
        o_md5, d_md5 = md5(original), md5(decompiled)
        def strs(p):
            try:
                out = subprocess.run(["strings", str(p)], capture_output=True, text=True, check=True)
                return set(out.stdout.splitlines())
            except subprocess.CalledProcessError as e:
                logger.warning(f"Strings command failed for {p}: {e.stderr}")
                return set() # Return empty set on error
            except FileNotFoundError:
                logger.error("'strings' command not found. Please ensure it's installed and in PATH.")
                return set()


        o_str, d_str = strs(original), strs(decompiled)
        common = o_str & d_str
        union_len = len(o_str | d_str)
        min_len = min(len(o_str), len(d_str))

        from difflib import SequenceMatcher
        # Limit read size for performance and to avoid memory issues with large files
        b1, b2 = original.read_bytes()[:min(10000, o_size)], decompiled.read_bytes()[:min(10000, d_size)]
        sim = SequenceMatcher(None, b1, b2).ratio()
        return {
            "file_size": {"original": o_size, "decompiled": d_size, "diff": size_diff, "diff_percent": ((size_diff / o_size) * 100) if o_size > 0 else 'N/A'},
            "md5": {"original": o_md5, "decompiled": d_md5, "match": o_md5==d_md5},
            "strings": {
                "original_count": len(o_str),
                "decompiled_count": len(d_str),
                "common_count": len(common),
                "unique_orig_count": len(o_str - d_str),
                "unique_decomp_count": len(d_str - o_str),
                "jaccard_index_percent": (len(common) / union_len * 100) if union_len > 0 else 0.0,
                "overlap_coefficient_percent": (len(common) / min_len * 100) if min_len > 0 else 0.0,
                "coverage_of_original_percent": (len(common) / len(o_str) * 100) if len(o_str) > 0 else (100.0 if not o_str and not d_str else 0.0),
            },
            "binary_similarity_first_10k_bytes": sim,
            "overall_assessment": {
                "md5_match": o_md5==d_md5,
                "size_closely_matches": size_diff < (o_size * 0.1) if o_size > 0 else d_size == 0, # e.g. within 10%
                "high_string_jaccard_index": ((len(common) / union_len * 100) if union_len > 0 else 0.0) > 70,
                "high_binary_similarity": sim > 0.7
            }
        }
    except FileNotFoundError as e:
        logger.error(f"Metrics generation file not found: {e}")
        return {"error": f"File not found: {e.filename}"}
    except Exception as e:
        logger.error(f"Metrics exception: {e}", exc_info=True)
        return {"error": str(e)}


# Create directories
CHAT_HISTORY_DIR = Path("chat_history")
CHAT_HISTORY_DIR.mkdir(exist_ok=True)
VISUALIZATION_DIR = Path("visualizations")
VISUALIZATION_DIR.mkdir(exist_ok=True)



def create_tool_call_graph(tool_calls: List[tuple], chat_id: str) -> Optional[str]:
    if not tool_calls:
        return None
    G = nx.DiGraph()

    def short(val):
        s = str(val)
        return s if len(s) < 40 else s[:37] + '...'

    # Set Graphviz attributes for clarity
    G.graph['graph'] = {'rankdir': 'LR'}
    G.graph['node'] = {'shape': 'box', 'style': 'filled', 'fillcolor': 'lightblue', 'fontname': 'Arial', 'fontsize': '10'}
    G.graph['edge'] = {'color': 'gray', 'arrowsize': '0.7'}

    for i, call_info in enumerate(tool_calls):
        tool_name = "Tool"
        tool_args_repr = ""
        if isinstance(call_info, tuple) and len(call_info) > 0:
            action_part = call_info[0]
            if hasattr(action_part, 'tool') and hasattr(action_part, 'tool_input'):
                tool_name = action_part.tool
                tool_args = action_part.tool_input
                if isinstance(tool_args, dict):
                    tool_args_repr = ", ".join(f"{k}={short(v)}" for k, v in tool_args.items() if k != 'explanation' and v is not None)
                elif isinstance(tool_args, (str, list, int, float, bool)):
                    tool_args_repr = short(tool_args)
        label = f"{tool_name}"
        if tool_args_repr:
            label += f" ({tool_args_repr})"
        G.add_node(i, label=label)
        if i > 0:
            G.add_edge(i-1, i)

    if not G.nodes:
        return None

    try:
        pos = graphviz_layout(G, prog="dot")
    except Exception as e:
        logger.warning(f"Graphviz layout failed: {e}, falling back to spring_layout.")
        pos = nx.spring_layout(G, seed=42)

    plt.figure(figsize=(max(8, len(G.nodes)*2), 4))
    nx.draw(G, pos, with_labels=True, labels=nx.get_node_attributes(G, 'label'),
            node_color='lightblue', edge_color='gray', node_size=2000, font_size=10, font_weight='bold', arrows=True)
    plt.title("Tool Call Flow", fontsize=14, fontweight='bold')
    plt.axis('off')
    plt.tight_layout(pad=1.0)
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S_%f")
    vis_path = VISUALIZATION_DIR / f"vis_{chat_id}_{timestamp}.png"
    try:
        plt.savefig(vis_path, format='png', bbox_inches='tight', dpi=120)
        logger.info(f"Visualization saved to {str(vis_path)}")
    except Exception as e_save_fig:
        logger.error(f"Failed to save visualization: {e_save_fig}")
        vis_path = None
    plt.close()
    return str(vis_path) if vis_path else None


# Initialize session state
if "messages" not in st.session_state: # For UI display
    st.session_state.messages = [{"role": "assistant", "content": "👋 Welcome! Select a file and click Analyze."}]
if "llm_history" not in st.session_state: # For Langchain agent
    st.session_state.llm_history = [AIMessage(content="👋 Welcome! Select a file and click Analyze.")]

for key in ("current_file", "agent", "r2_tools", "selected_file", "compilation_attempts", "tool_calls_display"):
    st.session_state.setdefault(key, None)
st.session_state.setdefault("show_advanced", False)
st.session_state.setdefault("generate_readme", True)
st.session_state.setdefault("compilation_attempts", 0)
st.session_state.setdefault("show_visualization", True)
st.session_state.setdefault("explain_in_detail", False)  # New state for detailed explanations
st.session_state.setdefault("tool_calls_display", []) # For storing intermediate steps for display with a message

if "current_chat_id" not in st.session_state:
    st.session_state.current_chat_id = f"chat_{datetime.now().strftime('%Y%m%d_%H%M%S_%f')}"

# Function to reconstruct llm_history from st.session_state.messages (UI dicts)
def reconstruct_llm_history_from_ui():
    st.session_state.llm_history = []
    for msg_dict in st.session_state.messages:
        if msg_dict["role"] == "user":
            st.session_state.llm_history.append(HumanMessage(content=msg_dict["content"]))
        elif msg_dict["role"] == "assistant":
            st.session_state.llm_history.append(AIMessage(content=msg_dict["content"]))
    logger.debug(f"LLM history reconstructed from UI messages. Count: {len(st.session_state.llm_history)}")


def save_chat_history(ui_messages, chat_id): # Save UI messages
    chat_file = CHAT_HISTORY_DIR / f"{chat_id}.json"
    serializable_messages = []
    for msg in ui_messages:
        s_msg = msg.copy()
        if "visualization_path" in s_msg and isinstance(s_msg["visualization_path"], Path):
            s_msg["visualization_path"] = str(s_msg["visualization_path"])
        # Ensure advanced_output is serializable if it contains non-standard objects
        if "advanced_output" in s_msg and s_msg["advanced_output"] is not None:
            try: # Quick check if it's JSON serializable
                json.dumps(s_msg["advanced_output"])
            except TypeError:
                logger.warning(f"Advanced output for a message in chat {chat_id} is not JSON serializable, converting to string.")
                s_msg["advanced_output"] = {"error": "Content not serializable", "original_type": str(type(s_msg["advanced_output"])), "string_repr": str(s_msg["advanced_output"])}

        serializable_messages.append(s_msg)

    chat_data = {
        "id": chat_id,
        "timestamp": datetime.now().strftime("%Y%m%d_%H%M%S"),
        "messages": serializable_messages, # UI messages
        "current_file": st.session_state.get("current_file"),
        "selected_file": st.session_state.get("selected_file"),
    }
    with open(chat_file, "w") as f:
        json.dump(chat_data, f, indent=2)
    logger.info(f"Chat history saved: {chat_file}")

def load_chat_history_data(chat_id):
    chat_file = CHAT_HISTORY_DIR / f"{chat_id}.json"
    if chat_file.exists():
        with open(chat_file, "r") as f:
            chat_data = json.load(f)
        logger.info(f"Chat history data loaded: {chat_file}")
        st.session_state.current_file = chat_data.get("current_file")
        st.session_state.selected_file = chat_data.get("selected_file")
        # UI messages are returned. LLM history will be reconstructed.
        return chat_data.get("messages", [])
    logger.warning(f"Chat history not found: {chat_file}")
    return None

def get_chat_histories():
    chats = []
    for chat_file in CHAT_HISTORY_DIR.glob("chat_*.json"):
        try:
            with open(chat_file, "r") as f:
                chat_data = json.load(f)
                preview_content = "Empty chat"
                if chat_data.get("messages"):
                    first_user_msg = next((m["content"] for m in chat_data["messages"] if m["role"] == "user"), None)
                    if first_user_msg: preview_content = first_user_msg
                    elif chat_data["messages"]: preview_content = chat_data["messages"][0]["content"]
                chats.append({
                    "id": chat_data.get("id", chat_file.stem),
                    "timestamp": chat_data.get("timestamp", chat_file.stem.split('_')[1] if '_' in chat_file.stem else "unknown"),
                    "preview": preview_content[:60] + "..." if preview_content else "Chat Session"
                })
        except json.JSONDecodeError: logger.error(f"Could not decode JSON: {chat_file}")
        except Exception as e: logger.error(f"Error reading chat file {chat_file}: {e}")
    return sorted(chats, key=lambda x: x["timestamp"], reverse=True)

def delete_chat_history(chat_id):
    chat_file = CHAT_HISTORY_DIR / f"{chat_id}.json"
    deleted_vis = False
    for vis_file in VISUALIZATION_DIR.glob(f"vis_{chat_id}_*.png"):
        try: vis_file.unlink(); logger.info(f"Deleted viz: {vis_file}"); deleted_vis = True
        except OSError as e: logger.error(f"Error deleting viz {vis_file}: {e}")
    if chat_file.exists():
        try: chat_file.unlink(); logger.info(f"Deleted chat: {chat_file}"); return True
        except OSError as e: logger.error(f"Error deleting chat file {chat_file}: {e}"); return False
    return deleted_vis

setup_logging()

# ―― Sidebar ――
st.sidebar.title("File System & Settings")
try:
    app_root = Path(__file__).parent
    default_cwd = app_root / "demo_code"
    if not default_cwd.exists(): default_cwd.mkdir(exist_ok=True, parents=True)
except NameError:
    default_cwd = Path.cwd() / "demo_code"
    if not default_cwd.exists(): default_cwd.mkdir(exist_ok=True, parents=True)

cwd_path_str = st.sidebar.text_input("Analysis Directory", value=str(default_cwd.resolve()))
cwd = Path(cwd_path_str)

llm_mode_sidebar = st.sidebar.radio("LLM Mode", ["openai","local"], index=0 if st.session_state.get("llm_mode_agent", "openai") == "openai" else 1)
st.session_state.llm_mode_agent = llm_mode_sidebar # Store selection for agent re-init

st.session_state.show_visualization = st.sidebar.checkbox("Show Tool Call Visualization", value=st.session_state.get("show_visualization", True))
st.session_state.show_advanced = st.sidebar.checkbox("Show Advanced Details", value=st.session_state.get("show_advanced", False))
st.session_state.generate_readme = st.sidebar.checkbox("Auto-Generate README on Compile", value=st.session_state.get("generate_readme",True))
st.session_state.explain_in_detail = st.sidebar.checkbox("Explain in Detail", value=st.session_state.get("explain_in_detail", False), 
    help="When enabled, the model will provide detailed explanations of the decompilation process in a markdown table format")

try:
    if cwd.exists() and cwd.is_dir():
        files = sorted([f.name for f in cwd.iterdir() if f.is_file() and not f.name.startswith('.')])
    else:
        files = ["(Invalid Directory)"]; st.sidebar.error(f"Directory not found: {cwd}")
    
    choice = st.sidebar.selectbox("Choose file to analyze", files if files else ["(No files in directory)"])

    if st.sidebar.button("Analyze Selected File", type="primary"):
        if choice and choice not in ["(Invalid Directory)", "(No files in directory)"]:
            st.session_state.current_file = str(cwd / choice)
            st.session_state.selected_file = choice
            
            initial_assistant_content = f"Ready to analyze `{choice}`. What would you like to do?"
            st.session_state.messages = [{"role": "assistant", "content": initial_assistant_content}]
            st.session_state.llm_history = [AIMessage(content=initial_assistant_content)] # Reset LLM history
            
            st.session_state.tool_calls_display = []
            st.session_state.compilation_attempts = 0
            st.session_state.current_chat_id = f"chat_{datetime.now().strftime('%Y%m%d_%H%M%S_%f')}"
            
            try:
                st.session_state.r2_tools = Radare2Tools(st.session_state.current_file, "standard")
                st.session_state.agent = ReverseEngineeringAgent(st.session_state.llm_mode_agent, "standard")
                logger.info(f"New analysis for {choice}. Chat ID: {st.session_state.current_chat_id}. Agent history initialized.")
            except Exception as e_init:
                logger.error(f"Error initializing agent/tools for {choice}: {e_init}", exc_info=True)
                st.error(f"Failed to initialize analysis tools for {choice}: {e_init}")
                st.session_state.agent = None; st.session_state.r2_tools = None
            st.rerun()
        else:
            st.sidebar.error("Please select a valid file from the directory.")
except Exception as e:
    st.sidebar.error(f"File system error: {e}")
    logger.error(f"Sidebar file error: {e}", exc_info=True)

# Chat History Management
st.sidebar.title("Chat Sessions")
chats = get_chat_histories()

if st.sidebar.button("Start New Chat Session"):
    welcome_msg = "👋 Welcome! Select a file to analyze, or load a previous chat session."
    st.session_state.messages = [{"role": "assistant", "content": welcome_msg}]
    st.session_state.llm_history = [AIMessage(content=welcome_msg)]
    st.session_state.current_chat_id = f"chat_{datetime.now().strftime('%Y%m%d_%H%M%S_%f')}"
    st.session_state.current_file = None; st.session_state.selected_file = None
    st.session_state.tool_calls_display = []; st.session_state.compilation_attempts = 0
    st.session_state.agent = ReverseEngineeringAgent(st.session_state.llm_mode_agent, "standard") # New agent for new chat
    st.session_state.r2_tools = None
    logger.info(f"New chat started. Chat ID: {st.session_state.current_chat_id}. Agent history reset.")
    st.rerun()

# Save current chat only if there's substantial content (more than just welcome)
if st.session_state.messages and (len(st.session_state.messages) > 1 or (len(st.session_state.messages)==1 and "Welcome" not in st.session_state.messages[0]['content'])):
    if st.sidebar.button("Save Current Session"):
        if not st.session_state.current_chat_id:
             st.session_state.current_chat_id = f"chat_{datetime.now().strftime('%Y%m%d_%H%M%S_%f')}"
        save_chat_history(st.session_state.messages, st.session_state.current_chat_id)
        st.sidebar.success(f"Session saved: {st.session_state.current_chat_id.split('_')[0]}_{st.session_state.current_chat_id.split('_')[1]}")
        st.rerun()

if chats:
    st.sidebar.write("Load Previous Session:")
    for chat_meta in chats:
        btn_label = f"{datetime.strptime(chat_meta['timestamp'], '%Y%m%d_%H%M%S').strftime('%b %d, %H:%M')} - {chat_meta['preview']}"
        col1, col2 = st.sidebar.columns([0.8, 0.2])
        with col1:
            if st.button(btn_label, key=f"load_{chat_meta['id']}", help="Load this chat session"):
                loaded_ui_messages = load_chat_history_data(chat_meta['id'])
                if loaded_ui_messages:
                    st.session_state.messages = loaded_ui_messages
                    st.session_state.current_chat_id = chat_meta['id']
                    reconstruct_llm_history_from_ui() # Key: Rebuild LLM history
                    st.session_state.tool_calls_display = []
                    st.session_state.compilation_attempts = 0

                    # Re-initialize agent and tools based on loaded file context
                    st.session_state.agent = ReverseEngineeringAgent(st.session_state.llm_mode_agent, "standard") # New agent instance
                    if st.session_state.current_file and Path(st.session_state.current_file).exists():
                        try:
                            st.session_state.r2_tools = Radare2Tools(st.session_state.current_file, "standard")
                            logger.info(f"Loaded chat {chat_meta['id']}. R2 tools re-initialized for {st.session_state.selected_file}.")
                        except Exception as e_load_r2:
                            logger.error(f"Error re-initializing R2 tools for loaded chat {chat_meta['id']}: {e_load_r2}")
                            st.warning(f"Could not set up R2 tools for {st.session_state.selected_file} from loaded chat.")
                            st.session_state.r2_tools = None
                    elif st.session_state.current_file: # File path exists in chat but not on disk
                        st.warning(f"File '{st.session_state.selected_file}' from loaded chat not found at '{st.session_state.current_file}'. Radare2 tools unavailable.")
                        st.session_state.r2_tools = None
                    else: # No file context
                        st.session_state.r2_tools = None
                    logger.info(f"Chat {chat_meta['id']} loaded. Agent LLM history reconstructed.")
                    st.rerun()
        with col2:
            if st.button("🗑️", key=f"delete_{chat_meta['id']}", help="Delete this chat session"):
                if delete_chat_history(chat_meta['id']):
                    st.sidebar.success(f"Deleted {chat_meta['id']}")
                    if st.session_state.current_chat_id == chat_meta['id']: # If current chat was deleted
                        welcome_msg_del = "👋 Current session deleted. Start a new one or load another."
                        st.session_state.messages = [{"role": "assistant", "content": welcome_msg_del}]
                        st.session_state.llm_history = [AIMessage(content=welcome_msg_del)]
                        st.session_state.current_chat_id = f"chat_{datetime.now().strftime('%Y%m%d_%H%M%S_%f')}"
                    st.rerun()
                else: st.sidebar.error(f"Failed to delete {chat_meta['id']}")

# ―― Main chat UI ――
st.title("Binary Analysis Assistant")

for idx, msg_ui in enumerate(st.session_state.messages):
    role, text_content = msg_ui["role"], msg_ui["content"]
    with st.chat_message(role):
        st.markdown(text_content)
        # Display visualization if path is stored with this UI message
        if role == "assistant" and st.session_state.show_visualization and msg_ui.get("visualization_path"):
            vis_path_str = msg_ui["visualization_path"]
            if Path(vis_path_str).exists():
                st.image(vis_path_str, caption="Tool Call Flow for this response", use_container_width='auto')
            else: st.caption(f"Missing visualization: {vis_path_str}")
        
        if st.session_state.show_advanced and msg_ui.get("advanced_output"):
            with st.expander("Advanced Details (Agent Response Structure)"): st.json(msg_ui["advanced_output"])

    # --- CODE-DETECTION & BUTTON LOGIC (for assistant messages) ---
    if role == "assistant":
        message_id = f"assistant_msg_{idx}_{st.session_state.current_chat_id}"
        has_code = "```c" in text_content
        
        if has_code:
            if st.button("Extract and Compile C Code", key=f"extract_{message_id}"):
                if not st.session_state.current_file:
                    st.error("Original file context missing. Re-analyze the file or load a chat with file context.")
                elif not st.session_state.agent:
                    st.error("Agent not initialized. Please analyze a file first.")
                else:
                    extracted_code = extract_c_code_from_markdown(text_content)
                    if extracted_code:
                        analysis_dir = Path(st.session_state.current_file).parent if st.session_state.current_file else Path(cwd)
                        out_stem_name = Path(st.session_state.selected_file if st.session_state.selected_file else "decompiled_code").stem
                        
                        # Base path for .c and binary (binary will not have .c suffix)
                        # Increment attempt number in filename to avoid overwriting
                        st.session_state.compilation_attempts += 1
                        output_base_path = analysis_dir / f"{out_stem_name}_decompiled_v{st.session_state.compilation_attempts}"

                        binp_path, compile_error_msg = compile_c_code(extracted_code, output_base_path, optimize=False)
                        
                        if binp_path:
                            # st.session_state.compilation_attempts = 0 # Reset on success for *this code block*
                            compile_success_text = f"✅ Code compiled successfully!\n   - C source: `{output_base_path.with_suffix('.c')}`\n   - Executable: `{binp_path}`"
                            st.session_state.messages.append({"role":"assistant", "content": compile_success_text})
                            st.session_state.llm_history.append(AIMessage(content=compile_success_text))

                            metrics_data = generate_metrics(Path(st.session_state.current_file), binp_path)
                            metrics_text = f"📊 Decompilation Metrics vs Original (`{st.session_state.selected_file}`):\n```json\n{json.dumps(metrics_data, indent=2)}\n```"
                            st.session_state.messages.append({"role":"assistant", "content": metrics_text})
                            st.session_state.llm_history.append(AIMessage(content=metrics_text))

                            if st.session_state.generate_readme:
                                with st.spinner("Generating README.md…"):
                                    try:
                                        readme_prompt_text = f"Generate a comprehensive README.md for the following C code which was the result of a decompilation. The original binary was '{st.session_state.selected_file}'. Focus on: project description, inferred functionality, build instructions (GCC, assuming the code provided), potential usage, and any observable security considerations from the code. Be concise and factual.\n\nDecompiled C Code:\n```c\n{extracted_code}\n```"
                                        
                                        # This is an agent turn for README
                                        st.session_state.llm_history.append(HumanMessage(content=readme_prompt_text))
                                        
                                        readme_agent_instance = st.session_state.agent.create_agent(st.session_state.r2_tools.get_tools() if st.session_state.r2_tools else [])
                                        readme_result = readme_agent_instance.invoke({
                                            "input": readme_prompt_text,
                                            "history": st.session_state.llm_history[:-1] # History before this README prompt
                                        })
                                        readme_content_raw = readme_result["output"]
                                        # Ensure README is plain markdown, no ```markdown wrapper
                                        if readme_content_raw.strip().startswith("```markdown"):
                                            readme_content_raw = readme_content_raw.split("```markdown",1)[1].rsplit("```",1)[0].strip()
                                        
                                        readme_filename = f"README_{out_stem_name}_v{st.session_state.compilation_attempts}.md"
                                        readme_file_path = analysis_dir / readme_filename
                                        readme_file_path.write_text(readme_content_raw)
                                        
                                        readme_gen_text = f"📝 Generated README.md: `{str(readme_file_path)}`\n\n---\n{readme_content_raw}"
                                        st.session_state.messages.append({"role":"assistant", "content": readme_gen_text})
                                        st.session_state.llm_history.append(AIMessage(content=readme_content_raw)) # Add the README content itself as AI msg
                                    except Exception as e_readme_gen:
                                        logger.error(f"README generation failed: {e_readme_gen}", exc_info=True)
                                        readme_fail_text = f"⚠️ README generation failed: {e_readme_gen}"
                                        st.session_state.messages.append({"role":"assistant", "content": readme_fail_text})
                                        st.session_state.llm_history.append(AIMessage(content=readme_fail_text))
                        else: # Compilation failed
                            error_header = f"❌ Compilation failed (attempt {st.session_state.compilation_attempts} for this code block):"
                            compile_fail_text_ui = f"{error_header}\n```\n{compile_error_msg}\n```"
                            st.session_state.messages.append({"role":"assistant", "content": compile_fail_text_ui})
                            st.session_state.llm_history.append(AIMessage(content=f"{error_header} {compile_error_msg}")) # Simpler for LLM history

                            if st.session_state.compilation_attempts < 3: # Max 2 AI fix attempts for this specific code block
                                with st.spinner("Attempting to fix compilation error with AI…"):
                                    try:
                                        fix_prompt_text = f"The following C code, which I attempted to compile as '{output_base_path.name}.c', failed with this GCC error: \n```\n{compile_error_msg}\n```\n\nPlease provide only the corrected C code, enclosed in a single markdown ```c ... ``` block. Do not include any explanations or apologies. Original C code was:\n```c\n{extracted_code}\n```"
                                        
                                        st.session_state.llm_history.append(HumanMessage(content=fix_prompt_text))
                                        
                                        fix_agent_instance = st.session_state.agent.create_agent(st.session_state.r2_tools.get_tools() if st.session_state.r2_tools else [])
                                        fix_result = fix_agent_instance.invoke({
                                            "input": fix_prompt_text,
                                            "history": st.session_state.llm_history[:-1]
                                        })
                                        fixed_code_suggestion_raw = fix_result["output"]
                                        
                                        ai_fix_text = f"🤖 AI suggested fix (attempt {st.session_state.compilation_attempts}):\n{fixed_code_suggestion_raw}"
                                        st.session_state.messages.append({"role":"assistant", "content": ai_fix_text})
                                        st.session_state.llm_history.append(AIMessage(content=fixed_code_suggestion_raw))
                                    except Exception as e_ai_fix:
                                        logger.error(f"AI fix attempt failed: {e_ai_fix}", exc_info=True)
                                        ai_fix_fail_text = f"⚠️ AI fix attempt (attempt {st.session_state.compilation_attempts}) failed: {e_ai_fix}"
                                        st.session_state.messages.append({"role": "assistant", "content": ai_fix_fail_text})
                                        st.session_state.llm_history.append(AIMessage(content=ai_fix_fail_text))
                            else:
                                max_attempts_msg = "Maximum AI fix attempts reached for this code block. Try fixing manually or ask for help."
                                st.session_state.messages.append({"role": "assistant", "content": max_attempts_msg})
                                st.session_state.llm_history.append(AIMessage(content=max_attempts_msg))
                    else:
                        st.warning("No C code found in the message to extract.")
                    st.rerun()


# ―― Chat input & agent invoke ――
user_input_text = st.chat_input("Ask about the binary or current analysis…")
if user_input_text:
    if not st.session_state.agent:
        st.error("Agent not initialized. Please analyze a file or start a new chat.")
    else:
        # Add detailed explanation prompt if enabled
        _user_input_text = user_input_text
        
        if st.session_state.explain_in_detail:
            detailed_prompt = """\n
After the task is complete, please provide a detailed explanation in a markdown table.
Required columns: Aspect, Explanation, Evidence

When explaining the decompilation approach, include the following aspects:
Decompilation Approach, Key Functions, Data Structures, Control Flow, Security Considerations

| Aspect | Explanation | Evidence |
|--------|-------------|----------|
| [Provide Aspect] | [Provide explanation] | [Provide specific evidence] |

Please ensure each row contains specific evidence from the binary analysis.
DO NOT include the table in README.md.
"""
            user_input_text = f"{_user_input_text}\n\n{detailed_prompt}"

        st.session_state.messages.append({"role":"user", "content": _user_input_text})
        st.session_state.llm_history.append(HumanMessage(content=user_input_text))
        with st.chat_message(st.session_state.messages[-1]["role"]):
            st.write(st.session_state.messages[-1]["content"])
        
        with st.spinner("Assistant is thinking…"):
            try:
                current_tools_list = st.session_state.r2_tools.get_tools() if st.session_state.r2_tools else []
                agent_executor_instance = st.session_state.agent.create_agent(current_tools_list)
                
                agent_response_struct = agent_executor_instance.invoke({
                    "input": user_input_text,
                    "history": st.session_state.llm_history[:-1]
                })
                
                assistant_final_response = agent_response_struct["output"]
                intermediate_tool_calls = agent_response_struct.get("intermediate_steps", [])
                
                # Prepare UI message dict for assistant
                assistant_ui_msg = {"role": "assistant", "content": assistant_final_response}
                if st.session_state.show_advanced:
                    assistant_ui_msg["advanced_output"] = {
                        "raw_agent_response": agent_response_struct
                    }
                
                if intermediate_tool_calls and st.session_state.show_visualization:
                    if not st.session_state.current_chat_id:
                        st.session_state.current_chat_id = f"chat_{datetime.now().strftime('%Y%m%d_%H%M%S_%f')}"
                    
                    generated_vis_path = create_tool_call_graph(intermediate_tool_calls, st.session_state.current_chat_id)
                    if generated_vis_path:
                        assistant_ui_msg["visualization_path"] = generated_vis_path
                    else:
                        logger.warning("Visualization generation returned None or path invalid.")

                st.session_state.messages.append(assistant_ui_msg)
                st.session_state.llm_history.append(AIMessage(content=assistant_final_response))
            
            except Exception as e_agent_invoke:
                logger.error(f"Agent invocation error: {e_agent_invoke}", exc_info=True)
                error_response_text = f"An error occurred during analysis: {str(e_agent_invoke)}"
                st.session_state.messages.append({"role": "assistant", "content": error_response_text})
                st.session_state.llm_history.append(AIMessage(content=error_response_text))

        # Auto-save chat after each interaction
        if len(st.session_state.messages) > 0 and st.session_state.current_chat_id:
            save_chat_history(st.session_state.messages, st.session_state.current_chat_id)
        st.rerun()