#include <assert.h>
#include <elf.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <syscall.h>
#include <unistd.h>
#include <signal.h>

/*
 * Fork a child process and set it up for tracing.
 * Replaces the child's image with the target program.
 */
pid_t run_target(char* const argv[])
{
    pid_t pid = fork();
    if (pid > 0) {
        return pid;
    } else if (pid == 0) {
        if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) < 0) {
            perror("ptrace");
            exit(1);
        }
        execv(argv[0], argv);
        exit(1);
    } else {
        perror("fork");
        exit(1);
    }
    return 0;
}

void* get_elf_content(const char* sym_name, const char* file_name)
{
    // Open ELF file for reading
    int elf_fd = open(file_name, O_RDONLY);
    if (elf_fd < 0) {
        perror("failed to open file");
        return NULL;
    }

    // Get file size
    struct stat elf_stats;
    int ret = fstat(elf_fd, &elf_stats);
    if (ret < 0) {
        perror("fstat failed");
        close(elf_fd);
        return NULL;
    }
    long file_size = elf_stats.st_size;

    // Allocate memory for the file content
    void* file_content = malloc(file_size);
    if (!file_content) {
        perror("malloc failed");
        close(elf_fd);
        return NULL;
    }

    // Read ELF file content into memory
    ret = read(elf_fd, file_content, file_size);
    if (ret < file_size) {
        perror("read failed or incomplete");
        free(file_content);
        close(elf_fd);
        return NULL;
    }
    close(elf_fd);

    return file_content;
}

unsigned long parse_elf(const char* target_sym, void* file_contents)
{
    // DONE: Look up target_sym address in the elf binary and return its virtual
    // address. Return 0 if not found.

    Elf64_Ehdr* ehdr = (Elf64_Ehdr*)file_contents;

    Elf64_Shdr* sections = (Elf64_Shdr*)((char*)file_contents + ehdr->e_shoff);

    for (int i = 0; i < ehdr->e_shnum; i++) {
        Elf64_Shdr* shdr = &sections[i];
    }

    for (int i = 0; i < ehdr->e_shnum; i++) {
        Elf64_Shdr* shdr = &sections[i];

        if (shdr->sh_type == SHT_SYMTAB) {
            Elf64_Sym* symtab = (Elf64_Sym*)((char*)file_contents + shdr->sh_offset);
            size_t num_symbols = shdr->sh_size / shdr->sh_entsize;

            Elf64_Shdr* strtab_hdr = &sections[shdr->sh_link];
            char* strtab = (char*)file_contents + strtab_hdr->sh_offset;

            for (size_t j = 0; j < num_symbols; j++) {
                Elf64_Sym* sym = &symtab[j];

                if (strcmp(strtab + sym->st_name, target_sym) == 0) {
                    printf("PRF:: symbol address is 0x%lx\n", sym->st_value); //part 1 print
                    unsigned char *f_comm = 
                        ((unsigned char*)(ehdr) + sections[sym->st_shndx].sh_offset 
                        + sym->st_value - sections[sym->st_shndx].sh_addr);
                    if(*f_comm == 0x55) //part 2 print
                        printf("PRF:: This function starts by pushing rbp\n");
                    return sym->st_value;
                }
            }
        }
    }
    printf("PRF:: symbol not found\n");
    return 0;
}

/*
 * Main debugger tracing loop.
 */

typedef struct{
    long addr;
    long instruct;
} pair;

typedef struct node{
    pair p;
    struct node *next;
} node;

typedef struct{
    node *first;
} CallStack;

void init(CallStack *stack) {stack->first = NULL;}
bool isEmpty(CallStack *stack) {return stack->first == NULL;}
void push(CallStack *stack, pair p){
    node *n = malloc(sizeof(node));
    n->p = p;
    n->next = stack->first;
    stack->first = n;
}
pair pop(CallStack *stack){
    if(isEmpty(stack)){
        pair p = {-1};
        return p;
    }
    node *temp = stack->first;
    stack->first = temp->next;
    pair p = temp->p;
    free(temp);
    return p;
}

CallStack stack;
long first_instruct;
void handle_break(int *num_rec, int *num_non_rec, int pid, unsigned long addr, int num); //done, not tested
void handle_enter(int *num_rec, int *num_non_rec, int pid, int num); //done, not tested
void handle_exit(int *num_rec, int pid);
void print_enter(bool rec, struct user_regs_struct *regs, int *num_non_rec, int num); //done, not tested

void run_tracer(pid_t child_pid, unsigned long addr, int nr_params)
{
    int wait_status, num_rec = 0, num_non_rec = 0;
    /*
    num_rec represents rcursive depth. for values > 0, num of recursive
    calls is (num_rec-1). there's also the first, nonrecursive call
    num_non_rec - a simple counter for non-recursive calls
    */
    struct user_regs_struct regs;
    init(&stack);
    // TODO: Implement tracing logic
    wait(&wait_status);
    first_instruct = ptrace(PTRACE_PEEKTEXT, child_pid, (void*)addr, (void*)0);
    long insert_first = (first_instruct & 0xFFFFFFFFFFFFFF00) | 0xCC;
    ptrace(PTRACE_POKETEXT, child_pid, (void*)addr, (void*)insert_first);

    while(1){
        wait(&wait_status);
        if(WIFEXITED(wait_status) || WIFSIGNALED(wait_status))
            return;
        if(WIFSTOPPED(wait_status)){
            if(WSTOPSIG(wait_status) == SIGTRAP){
                handle_break(&num_rec, &num_non_rec, child_pid, addr, nr_params);
                ptrace(PTRACE_CONT, child_pid, (void*)0, (void*)0);
            }
            else
                ptrace(PTRACE_CONT, child_pid, (void*)0, (void*)((long)WSTOPSIG(wait_status)));
        }
        else 
            ptrace(PTRACE_CONT, child_pid, (void*)0, (void*)0);
    }
}

void handle_break(int *num_rec, int *num_non_rec, int pid, unsigned long addr, int num){
    struct user_regs_struct regs;
    ptrace(PTRACE_GETREGS, pid, (void*)0, &regs);
    unsigned long curr_addr = regs.rip - 1;
    if(curr_addr == addr){
        //enter (the followed func was called)
        handle_enter(num_rec, num_non_rec, pid, num);
    }
    else{
        //exit
        handle_exit(num_rec, pid);
    }
}

void handle_enter(int *num_rec, int *num_non_rec, int pid, int num){
    struct user_regs_struct regs;
    ptrace(PTRACE_GETREGS, pid, NULL, &regs);
    print_enter(*num_rec, &regs, num_non_rec, num);
    (*num_rec)++;
    long ret_addr = ptrace(PTRACE_PEEKDATA, pid, (void*)regs.rsp, (void*)0);

    //save instruction
    pair p;
    p.addr = ret_addr;
    p.instruct = ptrace(PTRACE_PEEKTEXT, pid, (void*)ret_addr, (void*)0);
    push(&stack, p);

    //insert sigtrap to return
    long insert_first = (p.instruct & 0xFFFFFFFFFFFFFF00) | 0xCC;
    ptrace(PTRACE_POKETEXT, pid, (void*)ret_addr, (void*)insert_first);

    //do first func inst
    regs.rip--;
    ptrace(PTRACE_POKETEXT, pid, (void*)regs.rip, (void*)first_instruct);
    ptrace(PTRACE_SETREGS, pid, (void*)0, &regs);
    ptrace(PTRACE_SINGLESTEP, pid, (void*)0, (void*)0);
    waitpid(pid, NULL, 0);

    //intert sigtrap to func enter
    long first_insert = (first_instruct & 0xFFFFFFFFFFFFFF00) | 0xCC;
    ptrace(PTRACE_POKETEXT, pid, (void*)regs.rip, (void*)first_insert);
}


void handle_exit(int *num_rec, int pid){
    
}



void print_enter(bool rec, struct user_regs_struct *regs, int *num_non_rec, int num){
    if(num == 0){
        if(rec)
            printf("PRF:: entered recursive call with ()\n");
        else
            printf("PRF:: run #%d called with ():\n", *num_non_rec);
        return;
    }
    unsigned long long int arr[6];
    arr[0] = regs->rdi;
    arr[1] = regs->rsi;
    arr[2] = regs->rdx;
    arr[3] = regs->rcx;
    arr[4] = regs->r8;
    arr[5] = regs->r9;

    if(rec)
        printf("PRF:: entered recursive call with (");
    else
        printf("PRF:: run #%d called with (", *num_non_rec);
    for(int i = 0; i < num; i++){
        if(i != 0) printf(", ");
        printf("%llu", arr[i]);
    }
    printf(")\n");
}

int main(int argc, char* const argv[])
{
    if (argc < 4) {
        printf("usage: <sym_name> <number of input params> <elf_path> "
               "[optional input params for elf file]\n");
        return 0;
    }

    const char* sym_name  = argv[1];
    int nr_params         = strtol(argv[2], NULL, 10);
    const char* file_name = argv[3];

    // Put file content into memory for parsing
    void* file_content = get_elf_content(sym_name, file_name);

    // Find the symbol address
    unsigned long addr = parse_elf(sym_name, file_content);

    // Free the allocated memory
    free(file_content);

    //check if symbol was found. otherwise, exit
    if(addr == 0) return 1;

    // Launch the target program
    pid_t child_pid = run_target(argv + 3);

    // Run the tracer
    run_tracer(child_pid, addr, nr_params);

    return 0;
}
