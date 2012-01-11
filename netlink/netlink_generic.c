#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/socket.h>
#include <errno.h>

#define MAX_PAYLOAD 1024 // maximum payload size

int main(int argc, char* argv[])
{
    int state;
    struct sockaddr_nl src_addr, dest_addr;

    struct msghdr msg;
    struct nlmsghdr *nlh = NULL;
    struct iovec iov;

    int sock_fd, retval;
    int state_smg = 0;

    // Create a socket
    sock_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC); 
    if(sock_fd == -1){
        printf("error getting socket: %s", strerror(errno));
        return -1;
    }

    //set the src_addr to bind.
    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid(); // self pid
    src_addr.nl_groups = 1; // no broadcast group

    //start to bind.
    retval = bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr));
    if(retval < 0){
        printf("bind failed: %s", strerror(errno));
        close(sock_fd);
        return -1;
    }

    //prepare msghdr to sendmsg(, msghdr * , ).
    // !!!!! IN NETLINK: msghdr = &dest_addr + &(nlmsghdr + payload)
    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    if(!nlh){
        printf("malloc nlmsghdr error!\n");
        close(sock_fd);
        return -1;
    }
    
    //the dest_addr.
    memset(&dest_addr,0,sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0;
    dest_addr.nl_groups = 1;

    //the nlmsghdr + payload.
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD); //message total length
    nlh->nlmsg_pid = getpid(); //sending process PID
    printf("pid: %u\n", nlh->nlmsg_pid);
    nlh->nlmsg_flags = 0;  //?????
    strcpy(NLMSG_DATA(nlh),"Hello you!");  //here is message data.

    //load the nlmsghdr+payload to iovec
    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;

    //cast the msghdr structure.
    memset(&msg, 0, sizeof(msg));
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    printf("Start to send message...\n");
    state_smg = sendmsg(sock_fd,&msg,0);

    if(state_smg == -1)
    {
        printf("sendmsg error : %s\n",strerror(errno));
	goto out;
    }
    printf("message sent!(%s)\n", (char *) NLMSG_DATA((struct nlmsghdr *)msg.msg_iov->iov_base));
    printf("-----------------------------------\n");

    ///////////////////////////////////////////////////////////////////////
    // Read message from kernel
    memset(nlh,0,NLMSG_SPACE(MAX_PAYLOAD));
    printf("waiting received!\n");

    while(1){
        state = recvmsg(sock_fd, &msg, 0);
        if(state < 0)
        {
            printf("recvmsg error : %s\n",strerror(errno));
            break;
        }
        printf("Message received: %s\n",(char *) NLMSG_DATA(nlh));
    }

out:
    close(sock_fd);

    return 0;
}


