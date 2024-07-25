#include  <stdio.h>
#include  <unistd.h>
#include  <errno.h>

int main() {
    printf("Hello, this is the Parent process %d\n", getpid());
    printf("Here is the parent User id: %d\n", getuid());

    // input from the user, whether file or directory
    // char *args[] = {"/home/kali/Desktop/simple_slash/home/bob/bobdir/adir", "/home/kali/Desktop/simple_slash/change_dir", NULL};

    // execv("/home/kali/Desktop/simple_slash/simple_sudo", args);

    char *args[] = {"/home/kali/Desktop/simple_slash/home/alice/alice_data", NULL};

    execv("/home/kali/Desktop/simple_slash/setfacl", args);

    return 0;
}