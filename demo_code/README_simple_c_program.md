# SampleBinaryDecompiler

## Project Description
SampleBinaryDecompiler is a fully recovered C implementation of a previously opaque proprietary binary. This project restores the original logic, data structures, and algorithms in readable, maintainable source form. It serves both as a reference for reverse-engineering pipelines and as a working executable for file processing and cryptographic operations.

## Functionality
- Command-line interface for file input and output  
- Configurable XOR-based encryption/decryption  
- SHA-256 hashing of input data  
- User credential verification via salted hash comparison  
- Detailed logging of operations and error conditions  

## Build Instructions

Prerequisites  
• GCC (≥ 7.0) or Clang  
• OpenSSL development headers (for SHA-256)  
• Make (optional)

Steps  
1. Clone the repository  
2. From project root, run:  
      make  
   or manually compile:  
      gcc -std=c11 -O2 -Wall \  
        -I/usr/include/openssl \  
        main.c crypto.c auth.c logger.c \  
        -lcrypto -o samplebinary  

## Usage

Basic syntax:  
  samplebinary \  
    -i <input_file> \  
    -o <output_file> \  
    -k <key_file> \  
    -u <username> \  
    -p <password>

Options  
• -i, --input      Path to plaintext or ciphertext file  
• -o, --output     Destination path for processed output  
• -k, --key        Binary file containing XOR key bytes  
• -u, --user       Username for authentication  
• -p, --pass       Password for authentication  
• -h, --help       Display usage information

Examples  
• Encrypt:  
    samplebinary -i secret.txt -o secret.enc -k mykey.bin  
• Decrypt (same command):  
    samplebinary -i secret.enc -o secret.dec -k mykey.bin  
• Authenticate only:  
    samplebinary -u alice -p S3cur3P@ss

## Security Considerations
• Input validation: All file paths and user inputs must be sanitized to prevent path traversal.  
• Buffer boundaries: The implementation uses fixed-size buffers; potential overflow if inputs exceed expected length. Review and harden `read_file()` and `format_output()`.  
• Key management: XOR key is loaded into memory without secure wipe; consider using `OPENSSL_cleanse()` after use.  
• Authentication: Salt is hardcoded—replace with randomly generated per-user salts stored securely.  
• Logging: Sensitive data (passwords, keys) should never be written to logs. Adjust log levels and redact secrets.  

For full design rationale, refer to inline comments in source files.