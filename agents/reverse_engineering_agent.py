from typing import List, Dict, Any, Optional
from langchain_core.messages import HumanMessage, AIMessage, ToolMessage
from langchain_core.prompts import ChatPromptTemplate, MessagesPlaceholder
from langchain.agents.format_scratchpad.openai_tools import format_to_openai_tool_messages
from langchain.agents.output_parsers.openai_tools import OpenAIToolsAgentOutputParser
from langchain.agents import AgentExecutor
from langchain_openai import ChatOpenAI
from langchain_community.chat_models import ChatOllama
from langchain_core.callbacks import CallbackManager
import os

class ReverseEngineeringAgent:
    def __init__(self, llm_type: str = "local", analysis_mode: str = "standard"):
        self.llm_type = llm_type
        self.analysis_mode = analysis_mode
        self.llm = self._initialize_llm()
        self.system_prompt = self._get_system_prompt()
        self.history = []

    def _initialize_llm(self):
        """Initialize the LLM based on the specified type."""
        if self.llm_type == "local":
            return ChatOllama(
                model=os.getenv("OLLAMA_MODEL", "qwen2.5-coder:7b"),
                base_url=os.getenv("OLLAMA_BASE_URL", "http://localhost:11434")
            )
        else:
            return ChatOpenAI(
                model="o4-mini",
                api_key=os.getenv("OPENAI_REV_ENG_API_KEY")
            )

    def _get_system_prompt(self) -> str:
        """Get the system prompt based on analysis mode."""
        base_prompt = """You are an AI Software Analysis & Reverse Engineering Agent specialized in binary analysis and decompilation. Your mission is to decompile and semantically interpret binary files.

Use the language of the user to answer the question.
        
REQUIRED OUTPUT FORMAT:
1. Decompiled C Code:
   - Complete, compilable C source code
   - Well-structured and commented
   - Preserved variable names and types
   - Include necessary header files
   - Use markdown code blocks with language specifier

2. Semantic Analysis:
   - Brief program overview
   - Function purposes and behaviors
   - Key algorithms and data structures
   - Security implications if any

ANALYSIS STEPS:
1. Use analyze() for initial binary analysis
2. Use list_functions() to identify functions
3. Use decompile() for each function
4. Use search_strings() for string analysis
5. Use get_imports() and get_exports() for dependencies

ANALYSIS MODE: {analysis_mode}

When you are asked to fix a compilation error, you should:
- Check the error message
- Check the decompiled code
- Check the chat history
If necessary:
- Check the Radare2 tools output and adjust the decompiled code accordingly

README GENERATION GUIDELINES:
- Generate a comprehensive README.md for the decompiled code
- Try to not use any tools for this specific task if possible, mostly context from the chat history about the decompiled code
- Do not include ```markdown``` tags in the README
- Include as low amount of code as possible
- Include instructions
- Do not ask any questions to the user
- Do not include any other text than the README content

DO NOT:
- Include unnecessary explanations
- Add verbose descriptions
- Provide redundant information
- Include tool usage logs
- Add progress updates

FOCUS ON:
- Accurate decompilation
- Clear code structure
- Essential semantic analysis
- Security implications"""

        return base_prompt.format(analysis_mode=self.analysis_mode)

    def create_agent(self, tools: List[Any], callbacks: Optional[List[Any]] = None) -> AgentExecutor:
        """Create and configure the agent executor with optional callbacks."""
        prompt = ChatPromptTemplate.from_messages([
            ("system", self.system_prompt),
            ("user", "{input}"),
            MessagesPlaceholder(variable_name="agent_scratchpad"),
            MessagesPlaceholder(variable_name="history")
        ])

        # Create callback manager if callbacks are provided
        callback_manager = CallbackManager(callbacks) if callbacks else None

        # Bind tools and callbacks to LLM
        llm_with_tools = self.llm.bind_tools(tools)
        if callback_manager:
            llm_with_tools = llm_with_tools.with_config({"callbacks": callback_manager})

        agent = (
            {
                "input": lambda x: x["input"],
                "history": lambda x: x["history"],
                "agent_scratchpad": lambda x: format_to_openai_tool_messages(
                    x["intermediate_steps"]
                ),
            }
            | prompt
            | llm_with_tools
            | OpenAIToolsAgentOutputParser()
        )

        return AgentExecutor(
            agent=agent,
            tools=tools,
            verbose=True,
            return_intermediate_steps=True,
            callbacks=callbacks
        )

    def update_history(self, query: str, result: Dict[str, Any]) -> None:
        """Update the conversation history with the latest interaction."""
        self.history.append(HumanMessage(content=query))
        
        for action, observation in result.get("intermediate_steps", []):
            self.history.append(ToolMessage(
                tool_call_id=action.tool_call_id,
                content=str(observation)
            ))
            
        self.history.append(AIMessage(content=result["output"])) 