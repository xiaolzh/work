#define MAXBUF 1024
int SentToSever(int nPort, const char* pchIp, const char* sendContent, int len)
{
int sockfd;
struct sockaddr_in dest;
char buffer[MAXBUF];

if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
perror("Socket");
return -1;
}

bzero(&dest, sizeof(dest));
dest.sin_family = AF_INET;
dest.sin_port = htons(nPort);
if (inet_aton(pchIp, (struct in_addr *) &dest.sin_addr.s_addr) == 0) {
perror(pchIp);
return -1;
}

if (connect(sockfd, (struct sockaddr *) &dest, sizeof(dest)) != 0) {
perror("Connect ");
return -1;
}
int ret=send(sockfd, sendContent,len,0);

bzero(buffer, MAXBUF);
recv(sockfd, buffer, sizeof(buffer), 0);

close(sockfd);
return 0;
}
