import os
import streamlit as st
from pathlib import Path
from dotenv import load_dotenv
from loguru import logger
import json
from typing import Optional, List
import subprocess
import time

from tools.radare2_tools import Radare2Tools
from agents.reverse_engineering_agent import ReverseEngineeringAgent
from tools.code_generator import CodeGenerator
from langchain_core.messages import HumanMessage, AIMessage

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
            continue
        if line.strip().startswith("```") and in_block:
            in_block = False
            if current:
                code_blocks.append("\n".join(current))
                current = []
            continue
        if in_block:
            current.append(line)
    return "\n\n".join(code_blocks)

def compile_c_code(code: str, output_path: Path, optimize: bool = False) -> Optional[Path]:
    try:
        c_file = output_path.with_suffix(".c")
        with open(c_file, "w") as f:
            f.write(code)
        binary = output_path.with_suffix("")
        opt_flag = "-O2" if optimize else "-O0"
        cmd = ["gcc", opt_flag, "-o", str(binary), str(c_file)]
        res = subprocess.run(cmd, capture_output=True, text=True)
        if res.returncode != 0:
            logger.error(f"GCC error: {res.stderr}")
            return None
        return binary
    except Exception as e:
        logger.error(f"Compile exception: {e}")
        return None

def generate_metrics(original: Path, decompiled: Path) -> dict:
    try:
        o_size, d_size = original.stat().st_size, decompiled.stat().st_size
        size_diff = abs(o_size - d_size)
        import hashlib
        def md5(p): return hashlib.md5(p.read_bytes()).hexdigest()
        o_md5, d_md5 = md5(original), md5(decompiled)
        def strs(p):
            out = subprocess.run(["strings", str(p)], capture_output=True, text=True)
            return set(out.stdout.splitlines())
        o_str, d_str = strs(original), strs(decompiled)
        common = o_str & d_str
        total = len(o_str) + len(d_str)
        coverage = (len(common)*2/total*100) if total else 0.0
        from difflib import SequenceMatcher
        b1, b2 = original.read_bytes()[:1000], decompiled.read_bytes()[:1000]
        sim = SequenceMatcher(None, b1, b2).ratio()
        return {
            "file_size": {"original": o_size, "decompiled": d_size, "diff": size_diff},
            "md5": {"o": o_md5, "d": d_md5, "match": o_md5==d_md5},
            "strings": {
                "common": len(common),
                "unique_orig": len(o_str - d_str),
                "unique_decomp": len(d_str - o_str),
                "coverage_percent": coverage
            },
            "binary_similarity": sim,
            "overall": {
                "md5_match": o_md5==d_md5,
                "size_match": o_size==d_size,
                "high_string_cov": coverage>80,
                "high_bin_sim": sim>0.8
            }
        }
    except Exception as e:
        logger.error(f"Metrics exception: {e}")
        return {}

# Initialize session state
if "messages" not in st.session_state:
    st.session_state.messages = [{
        "role": "assistant",
        "content": "👋 Welcome! Select a file and click Analyze."
    }]
    st.session_state.pending_code_messages = set()
    st.session_state.force_rerun = False

for key in ("current_file","agent","r2_tools","show_advanced","save_output","output_filename","decompiled_files","selected_file"):
    st.session_state.setdefault(key, None)
st.session_state.setdefault("show_advanced", False)
st.session_state.setdefault("save_output", False)

# Create a container for forcing reruns
rerun_container = st.empty()

setup_logging()

# ―― Sidebar ――
st.sidebar.title("File System")
cwd = st.sidebar.text_input("Current Directory", value=str(Path.cwd() / "demo_code"))
st.sidebar.title("Analysis Settings")
llm_mode = st.sidebar.radio("LLM Mode", ["local","openai"])
st.session_state.show_advanced = st.sidebar.checkbox("Show Advanced", value=False)
st.session_state.save_output = st.sidebar.checkbox("Save Output", value=False)
if st.session_state.save_output:
    st.session_state.output_filename = st.sidebar.text_input("Filename", "analysis.txt")

try:
    files = os.listdir(cwd) # For testing purposes
    
    choice = st.sidebar.selectbox("Choose file", files)
    if st.sidebar.button("Analyze File"):
        st.session_state.current_file = str(Path(cwd)/choice)
        st.session_state.selected_file = choice
        st.session_state.messages = []
        st.session_state.r2_tools = Radare2Tools(st.session_state.current_file, "standard")
        st.session_state.agent = ReverseEngineeringAgent(llm_mode, "standard")
        st.session_state.messages.append({
            "role":"assistant",
            "content": f"Ready to analyze {choice}. What do you want?"
        })
except Exception as e:
    st.sidebar.error(f"Dir error: {e}")

# ―― Main chat UI ――
st.title("Binary Analysis Chat")

# Display chat messages
for idx, msg in enumerate(st.session_state.messages):
    role, text = msg["role"], msg["content"]
    with st.chat_message(role):
        st.write(text)
        if st.session_state.show_advanced and msg.get("advanced_output"):
            with st.expander("Advanced"):
                st.json(msg["advanced_output"])

    # --- CODE-DETECTION & BUTTON LOGIC ---
    message_id = f"{role}_{idx}"
    has_code = "```c" in text
    
    # Force rerun if we detect new code and haven't shown the button yet
    if has_code and message_id not in st.session_state.pending_code_messages:
        st.session_state.pending_code_messages.add(message_id)
        st.session_state.force_rerun = True
        rerun_container.empty()  # This will trigger a rerun
        st.rerun()
    
    # Show button if code exists
    if has_code:
        if st.button("Extract and Compile Code", key=f"extract_{message_id}"):
            code = extract_c_code_from_markdown(text)
            if code:
                # save .c file
                out_c = Path(cwd)/f"decompiled_{st.session_state.selected_file}.c"
                out_c.write_text(code)
                binp = compile_c_code(code, out_c, optimize=False)
                if binp:
                    metrics = generate_metrics(Path(st.session_state.current_file), binp)
                    st.session_state.messages.append({
                        "role":"assistant",
                        "content": f"✅ Compiled to {binp}"
                    })
                    
                    # Add metrics summary to AI history
                    metrics_summary = {
                        "file_size": "Compares the size of original and decompiled binaries",
                        "md5": "Checks if the binaries are identical using MD5 hash",
                        "strings": "Analyzes common and unique strings between binaries",
                        "binary_similarity": "Measures how similar the binary contents are",
                        "overall": "Combined assessment of binary similarity"
                    }
                    
                    st.session_state.messages.append({
                        "role":"assistant",
                        "content": "📊 Metrics Summary:\n" + 
                                  "\n".join([f"- {k}: {v}" for k, v in metrics_summary.items()]) +
                                  "\n\nDetailed Decompilation Metrics:\n```json\n" +
                                  json.dumps(metrics, indent=2) +
                                  "\n```"
                    })
                else:
                    st.session_state.messages.append({
                        "role":"assistant",
                        "content": "❌ Compilation failed."
                    })
            st.rerun()

# Reset force_rerun flag after processing
if st.session_state.force_rerun:
    st.session_state.force_rerun = False

# ―― Chat input & agent invoke ――
user_input = st.chat_input("Ask about the binary…")
if user_input:
    if not st.session_state.current_file:
        st.error("Pick a file first!")
    else:
        st.session_state.messages.append({"role":"user","content":user_input})
        with st.chat_message("user"):
            st.write(user_input)

        # show status
        status = st.empty()
        with status.container():
            with st.status("Running…"):
                # build & call
                agent = st.session_state.agent.create_agent(
                    st.session_state.r2_tools.get_tools()
                )
                result = agent.invoke({
                    "input": user_input,
                    "history": st.session_state.agent.history
                })
        status.empty()

        # append assistant response
        assistant_msg = {"role":"assistant","content":result["output"]}
        if st.session_state.show_advanced:
            assistant_msg["advanced_output"] = {
                "agent_calls": result.get("intermediate_steps", []),
                "metadata": result.get("metadata", {})
            }
        st.session_state.messages.append(assistant_msg)

        with st.chat_message("assistant"):
            st.write(result["output"])
            if st.session_state.show_advanced:
                with st.expander("Advanced"):
                    st.json(assistant_msg["advanced_output"])

        # save if requested
        if st.session_state.save_output and st.session_state.output_filename:
            outp = Path(cwd)/st.session_state.output_filename
            outp.write_text(result["output"])
            st.success(f"Saved to {outp}")
