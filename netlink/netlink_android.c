#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <fcntl.h>



static int device_fd = -1;


print_msg(char* msg, int len)
{
    int i = 0;

    while(i < len){
        if(msg[i] != 0)
            putchar(msg[i]);
        else
            putchar(' ');
        i++;
    }

    printf("\n");
}

static int open_uevent_socket(void)
{
    struct sockaddr_nl addr;
    int sz = 64*1024; // XXX larger? udev uses 16MB!
    int on = 1;
    int s;

    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_pid = getpid();
    addr.nl_groups = 0xffffffff;

    s = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if(s < 0)
        return -1;

    setsockopt(s, SOL_SOCKET, SO_RCVBUFFORCE, &sz, sizeof(sz));
    setsockopt(s, SOL_SOCKET, SO_PASSCRED, &on, sizeof(on));

    if(bind(s, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        close(s);
        return -1;
    }

    return s;
}


#define UEVENT_MSG_LEN  1024
void handle_device_fd()
{
    for(;;) {
        char msg[UEVENT_MSG_LEN+2];
        char cred_msg[CMSG_SPACE(sizeof(struct ucred))];
        struct iovec iov = {msg, sizeof(msg)};
        struct sockaddr_nl snl;
        struct msghdr hdr = {&snl, sizeof(snl), &iov, 1, cred_msg, sizeof(cred_msg), 0};

        ssize_t n = recvmsg(device_fd, &hdr, 0);
        if (n <= 0) {
            break;
        }

        if ((snl.nl_groups != 1) || (snl.nl_pid != 0)) {
            /* ignoring non-kernel netlink multicast message */
            //printf("ignoring non-kernel netlink multicast message, (nl_groups, nl_pid): %d, %u \n", snl.nl_groups, snl.nl_pid);
            //printf("-------msg received: %s\n", msg);
            continue;
        }

        struct cmsghdr * cmsg = CMSG_FIRSTHDR(&hdr);
        if (cmsg == NULL || cmsg->cmsg_type != SCM_CREDENTIALS) {
            /* no sender credentials received, ignore message */
            printf("no sender credentials received, ignore message\n");
            continue;
        }

        struct ucred * cred = (struct ucred *)CMSG_DATA(cmsg);
        if (cred->uid != 0) {
            /* message from non-root user, ignore */
            printf("message from non-root user, ignore\n");
            continue;
        }

        if(n >= UEVENT_MSG_LEN)   /* overflow -- discard */
            continue;

        msg[n] = '\0';
        msg[n+1] = '\0';
        
        //printf("msg received(%d): %s\n", n, msg);
        print_msg(msg, n);
    }
}


int main(int argc, char* argv[])
{
    struct stat info;
    int fd;

    device_fd = open_uevent_socket();
    if(device_fd < 0)
        return;

    fcntl(device_fd, F_SETFD, FD_CLOEXEC);
    //fcntl(device_fd, F_SETFL, O_NONBLOCK);

    while(1)
    {
        handle_device_fd();
    }
}
