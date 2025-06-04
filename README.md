# LLM Reverse Engineering Tool

A powerful reverse engineering tool that combines LLM capabilities with radare2 for binary analysis and decompilation.

## Features

- Multiple analysis modes (quick, standard, deep)
- Support for both local (Ollama) and cloud-based (OpenAI) LLMs
- Comprehensive radare2 integration
- Detailed logging and history tracking
- CLI interface with rich output formatting

## Prerequisites

### System Requirements

- Python 3.8 or higher
- radare2 installed on your system
- Ollama (if using local LLM)
- OpenAI API key (if using OpenAI)

### Installing radare2

#### Ubuntu/Debian:

```bash
sudo apt update
sudo apt install radare2
```

#### macOS:

```bash
brew install radare2
```

#### Windows (WSL):

```bash
sudo apt update
sudo apt install radare2
```

### Installing Ollama (for local LLM)

1. Download and install Ollama from [ollama.ai](https://ollama.ai)
2. Pull the required model:

```bash
ollama pull qwen2.5-coder:7b
```

## Installation

1. Clone the repository:

```bash
git clone <repository-url>
cd llm-reverse-engineering
```

2. Create and activate a virtual environment:

```bash
python -m venv .venv
source .venv/bin/activate  # On Windows: .venv\Scripts\activate
```

3. Install dependencies:

```bash
pip install -r requirements.txt
```

4. Copy the example environment file and configure it:

```bash
cp .env.example .env
# Edit .env with your configuration
```

## Configuration

The tool can be configured through environment variables in the `.env` file:

- `OPENAI_API_KEY`: Your OpenAI API key (required for OpenAI LLM)
- `OLLAMA_BASE_URL`: Base URL for Ollama server (default: http://localhost:11434)
- `OLLAMA_MODEL`: Model to use with Ollama (default: qwen2.5-coder:7b)
- `DEFAULT_ANALYSIS_MODE`: Default analysis mode (quick/standard/deep)
- `LOG_LEVEL`: Logging level (INFO/DEBUG/ERROR)
- `LOG_FILE`: Path to log file

## Usage

### Basic Usage

```bash
python main.py <path-to-binary>
```

### Advanced Options

```bash
python main.py <path-to-binary> --mode [quick|standard|deep] --llm-type [local|openai]
```

### Examples

1. Quick analysis with local LLM:

```bash
python main.py ./test_binary --mode quick --llm-type local
```

2. Deep analysis with OpenAI:

```bash
python main.py ./test_binary --mode deep --llm-type openai
```

### Analysis Modes

- `quick`: Basic analysis with minimal tool usage

  - Basic function listing
  - String search
  - Import/export analysis

- `standard`: Comprehensive analysis (default)

  - All quick mode features
  - Function decompilation
  - Memory analysis
  - Detailed string analysis

- `deep`: In-depth analysis
  - All standard mode features
  - Extended memory analysis
  - Cross-reference analysis
  - Pattern matching

## Project Structure

```
llm-reverse-engineering/
├── agents/
│   └── reverse_engineering_agent.py  # LLM agent implementation
├── tools/
│   └── radare2_tools.py             # Radare2 integration
├── config/
├── utils/
├── logs/                            # Analysis logs
├── main.py                          # CLI entry point
├── requirements.txt                 # Python dependencies
├── .env.example                    # Example configuration
└── README.md
```

## Troubleshooting

1. **Radare2 not found**

   - Ensure radare2 is installed and in your PATH
   - Verify installation with `r2 -v`

2. **Ollama connection issues**

   - Check if Ollama is running: `ollama list`
   - Verify the model is downloaded: `ollama list`
   - Check the OLLAMA_BASE_URL in .env

3. **OpenAI API issues**
   - Verify your API key in .env
   - Check your OpenAI account status
   - Ensure you have sufficient credits

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

This project is licensed under the MIT License - see the LICENSE file for details.
