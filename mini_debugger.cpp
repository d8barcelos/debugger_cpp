#include <iostream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/personality.h>
#include <sys/user.h>
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string>
#include <sstream>
#include <iomanip>

#include <cstdint>
class Debugger {
private:
    pid_t m_pid;
    std::string m_prog_name;
    
    void handle_command(const std::string& line) {
        auto args = split(line, ' ');
        std::string command = args[0];

        if (command == "continue") {
            continue_execution();
        }
        else if (command == "break") {
            if (args.size() != 2) {
                std::cout << "break command requires an address\n";
                return;
            }
            std::string addr_str {args[1]};
            uint64_t addr = std::stol(addr_str, 0, 16);
            set_breakpoint(addr);
        }
        else if (command == "registers") {
            print_registers();
        }
        else if (command == "memory") {
            if (args.size() != 3) {
                std::cout << "memory command requires address and length\n";
                return;
            }
            uint64_t addr = std::stol(args[1], 0, 16);
            uint64_t len = std::stol(args[2]);
            dump_memory(addr, len);
        }
        else {
            std::cout << "Unknown command\n";
        }
    }

    void continue_execution() {
        ptrace(PTRACE_CONT, m_pid, nullptr, nullptr);
        int wait_status;
        auto options = 0;
        waitpid(m_pid, &wait_status, options);
    }

    uint64_t get_pc() {
        user_regs_struct regs;
        ptrace(PTRACE_GETREGS, m_pid, nullptr, &regs);
        return regs.rip;
    }

    void set_pc(uint64_t pc) {
        user_regs_struct regs;
        ptrace(PTRACE_GETREGS, m_pid, nullptr, &regs);
        regs.rip = pc;
        ptrace(PTRACE_SETREGS, m_pid, nullptr, &regs);
    }

    uint64_t read_memory(uint64_t address) {
        return ptrace(PTRACE_PEEKDATA, m_pid, address, nullptr);
    }

    void write_memory(uint64_t address, uint64_t value) {
        ptrace(PTRACE_POKEDATA, m_pid, address, value);
    }

    void dump_memory(uint64_t address, size_t length) {
        std::cout << std::hex << std::setfill('0');
        for (size_t i = 0; i < length; i += 8) {
            uint64_t value = read_memory(address + i);
            std::cout << std::setw(16) << address + i << ": ";
            std::cout << std::setw(16) << value << "\n";
        }
    }

    void print_registers() {
        user_regs_struct regs;
        ptrace(PTRACE_GETREGS, m_pid, nullptr, &regs);
        
        std::cout << std::hex << std::setfill('0');
        std::cout << "RAX: 0x" << std::setw(16) << regs.rax << "\n";
        std::cout << "RBX: 0x" << std::setw(16) << regs.rbx << "\n";
        std::cout << "RCX: 0x" << std::setw(16) << regs.rcx << "\n";
        std::cout << "RDX: 0x" << std::setw(16) << regs.rdx << "\n";
        std::cout << "RDI: 0x" << std::setw(16) << regs.rdi << "\n";
        std::cout << "RSI: 0x" << std::setw(16) << regs.rsi << "\n";
        std::cout << "RBP: 0x" << std::setw(16) << regs.rbp << "\n";
        std::cout << "RSP: 0x" << std::setw(16) << regs.rsp << "\n";
        std::cout << "RIP: 0x" << std::setw(16) << regs.rip << "\n";
        std::cout << "R8:  0x" << std::setw(16) << regs.r8  << "\n";
        std::cout << "R9:  0x" << std::setw(16) << regs.r9  << "\n";
        std::cout << "R10: 0x" << std::setw(16) << regs.r10 << "\n";
        std::cout << "R11: 0x" << std::setw(16) << regs.r11 << "\n";
        std::cout << "R12: 0x" << std::setw(16) << regs.r12 << "\n";
        std::cout << "R13: 0x" << std::setw(16) << regs.r13 << "\n";
        std::cout << "R14: 0x" << std::setw(16) << regs.r14 << "\n";
        std::cout << "R15: 0x" << std::setw(16) << regs.r15 << "\n";
        std::cout << std::dec;
    }

    void set_breakpoint(uint64_t addr) {
        // Save original instruction
        uint64_t data = read_memory(addr);
        
        // Replace first byte with int3
        uint64_t int3 = 0xcc;
        uint64_t masked = (data & ~0xFF) | int3;
        
        write_memory(addr, masked);
        std::cout << "Breakpoint set at 0x" << std::hex << addr << std::dec << "\n";
    }

    std::vector<std::string> split(const std::string &s, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(s);
        while (std::getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

public:
    Debugger(std::string prog_name, pid_t pid) 
        : m_prog_name{std::move(prog_name)}, m_pid{pid} {}

    void run() {
        int wait_status;
        auto options = 0;
        waitpid(m_pid, &wait_status, options);

        char* line = nullptr;
        size_t len = 0;
        while(true) {
            std::cout << "debugger> ";
            if (getline(&line, &len, stdin) == -1) break;
            
            std::string command = std::string(line);
            command.pop_back(); // remove newline
            
            handle_command(command);
        }
        free(line);
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Program name not specified\n";
        return -1;
    }

    auto prog = argv[1];
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        personality(ADDR_NO_RANDOMIZE);
        ptrace(PTRACE_TRACEME, 0, nullptr, nullptr);
        execl(prog, prog, nullptr);
    }
    else if (pid >= 1) {
        // Parent process
        std::cout << "Started debugging process " << pid << "\n";
        Debugger dbg{prog, pid};
        dbg.run();
    }
}