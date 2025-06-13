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

## Docker Installation

You can also run the application using Docker, which eliminates the need for local installation of dependencies.

### Running from Docker Hub

To run the container directly from Docker Hub, use:

```bash
docker run -p 8501:8501 \
          -v $(pwd)/input:/app/input \
          -e OPENAI_REV_ENG_API_KEY=your-api-key \
          michaljurzak/llm-reverse-engineering:latest
```

Where `$(pwd)` input is your current directory mounted into the /app directory.

Please either paste your OpenAI API key or use from your environment variable (Windows: `$env:OPENAI_API_KEY`, Linux/MacOS: `$OPENAI_API_KEY`)

Access the Streamlit interface at http://localhost:8501

### Building the Docker Image

```bash
docker build -t reverse-engineering-tool .
```

### Running with Docker

Basic usage:

```bash
docker run -p 8501:8501 -v $(pwd)/input:/app/input -e OPENAI_REV_ENG_API_KEY=your-api-key llm-reverse-engineering
```

You can use your environment variable by using `$OPENAI_API_KEY` as a variable passed in a command in Linux/MacOS. in Windows use: `$env:OPENAI_API_KEY`

Advanced usage with environment variables and output directory:

```bash
docker run -p 8501:8501 \
          -v $(pwd)/input:/app/input \
          -v $(pwd)/output:/app/output \
          -e OPENAI_REV_ENG_API_KEY=your-api-key \
          -e LOG_LEVEL=INFO \
          llm-reverse-engineering
```

After running the container, you can access the Streamlit interface at http://localhost:8501

### Docker Volume Mapping

- `/app/input`: Mount your input directory containing binary files
- `/app/output`: Mount your output directory for analysis results
- `/app/logs`: Mount your logs directory (optional)
- `/app/generated_codes`: Mount your generated codes directory (optional)

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
