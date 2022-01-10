#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include "threadpool.h"

#define BUFLEN 4000
#define readdd 500
int checkPermission(char *path)
{

    int stat_status = 0;
    int dir_status = 0;
    if (stat_status == -1231 || dir_status == -123123)
        printf("\r");
    int r_permissions = -1;
    int x_permissions = -1;
    int slash_flag = 0;
    int break_flag = 0;
    int stop_flag = 0;
    struct stat fs;
    stat_status = stat(path, &fs);
    char copy[4000];
    copy[0] = 0;
    strcpy(copy, path);
    int i = strlen(copy) - 1;
    r_permissions = fs.st_mode & S_IROTH;
    if (r_permissions == 0)
        return -1;
    if (strcmp(path, "./") == 0)
        return 0;
    while (stop_flag == 0)
    {
        while ((copy[i] != '/' && i > 0) || slash_flag == 0)
        {
            slash_flag = 1;
            copy[i] = 0;
            i--;
            if (copy[i] == '/' && slash_flag == 1)
            {
                if (strcmp(copy, "./") == 0)
                {
                    copy[0] = 0;
                    copy[1] = 0;
                    break_flag = 1;
                    break;
                }
                else
                    break;
            }
        }
        if (break_flag == 1)
        {
            stop_flag = 1;
            break;
        }
        stat_status = stat(copy, &fs);
        x_permissions = fs.st_mode & S_IXOTH;
        if (x_permissions == 0)
            return -1;
        slash_flag = 0;
    }
    return 0;
}
int isNum(char *str)
{
    int i = 0;
    for (i = 0; i < strlen(str); i++) // Check every character in the string
    {
        if (isdigit(str[i]) == 0) // If the character is not between 1-9
        {
            return -1;
        }
    }
    return 0;
}

void addStr(char **str, char *concat) // Add string to an other without consider the size
{
    int counter = 0;
    counter = strlen(concat) + strlen(*str) + 1;
    *str = (char *)realloc(*str, counter);
    strcat(*str, concat);
}

char *get_mime_type(char *name)
{
    char *ext = strrchr(name, '.');
    if (!ext)
        return NULL;
    if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0)
        return "text/html";
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0)
        return "image/jpeg";
    if (strcmp(ext, ".gif") == 0)
        return "image/gif";
    if (strcmp(ext, ".png") == 0)
        return "image/png";
    if (strcmp(ext, ".css") == 0)
        return "text/css";
    if (strcmp(ext, ".au") == 0)
        return "audio/basic";
    if (strcmp(ext, ".wav") == 0)
        return "audio/wav";
    if (strcmp(ext, ".avi") == 0)
        return "video/x-msvideo";
    if (strcmp(ext, ".mpeg") == 0 || strcmp(ext, ".mpg") == 0)
        return "video/mpeg";
    if (strcmp(ext, ".mp3") == 0)
        return "audio/mpeg";
    return NULL;
}

char *send302(char *path)
{
    char *location = (char *)calloc(1, sizeof(char));
    addStr(&location, "Location: ");
    addStr(&location, path + 1);
    addStr(&location, "/\r\n");
    char date[1000];
    date[0] = 0;
    time_t now = time(0);
    struct tm tm = *gmtime(&now);
    strftime(date, sizeof date, "%a, %d %b %Y %H:%M:%S %Z", &tm);
    strcat(date, "\r\n");
    char *toReturn = (char *)calloc(1, sizeof(char));
    addStr(&toReturn, "HTTP/1.1 302 Found\r\n");
    addStr(&toReturn, "Server: webserver /1.0\r\n");
    addStr(&toReturn, date);
    addStr(&toReturn, location);
    addStr(&toReturn, "Content-Type: text/html\r\n");
    addStr(&toReturn, "Content-Length: 123\r\n");
    addStr(&toReturn, "Connection: close\r\n\r\n");
    addStr(&toReturn, "<HTML><HEAD><TITLE>302 Found</TITLE></HEAD>\r\n<BODY><H4>302 Found</H4>\r\nDirectories must end with a slash.\r\n</BODY></HTML>\r\n");
    free(location);
    return toReturn;
}

char *sendError(char *err)
{
    char date[1000];
    date[0] = 0;
    time_t now = time(0);
    struct tm tm = *gmtime(&now);
    strftime(date, sizeof date, "%a, %d %b %Y %H:%M:%S %Z", &tm);
    strcat(date, "\r\n");
    if (strcmp(err, "400") == 0)
    {
        char *toReturn = (char *)calloc(1, sizeof(char));
        addStr(&toReturn, "HTTP/1.1 Bad Request\r\n");
        addStr(&toReturn, "Server: webserver /1.0\r\n");
        addStr(&toReturn, date);
        addStr(&toReturn, "Content-Type: text/html\r\n");
        addStr(&toReturn, "Content-Length: 113\r\n");
        addStr(&toReturn, "Connection: close\r\n\r\n");
        addStr(&toReturn, "<HTML><HEAD><TITLE>400 Bad Request</TITLE></HEAD>\r\n<BODY><H4>400 Bad request</H4>\r\nBad Request.\r\n</BODY></HTML>\r\n");
        return toReturn;
    }
    else if (strcmp(err, "500") == 0)
    {
        char *toReturn = (char *)calloc(1, sizeof(char));
        addStr(&toReturn, "HTTP/1.1 500 Internal Server Error\r\n");
        addStr(&toReturn, "Server: webserver /1.0\r\n");
        addStr(&toReturn, date);
        addStr(&toReturn, "Content-Type: text/html\r\n");
        addStr(&toReturn, "Content-Length: 144\r\n");
        addStr(&toReturn, "Connection: close\r\n\r\n");
        addStr(&toReturn, "<HTML><HEAD><TITLE>500 Internal Server Error</TITLE></HEAD>\r\n<BODY><H4>500 Internal Server Error</H4>\r\nSome server side error.\r\n</BODY></HTML>\r\n");
        return toReturn;
    }
    else if (strcmp(err, "501") == 0)
    {
        char *toReturn = (char *)calloc(1, sizeof(char));
        addStr(&toReturn, "HTTP/1.1 501 Not supported\r\n");
        addStr(&toReturn, "Server: webserver /1.0\r\n");
        addStr(&toReturn, date);
        addStr(&toReturn, "Content-Type: text/html\r\n");
        addStr(&toReturn, "Content-Length: 129\r\n");
        addStr(&toReturn, "Connection: close\r\n\r\n");
        addStr(&toReturn, "<HTML><HEAD><TITLE>501 Not supported</TITLE></HEAD>\r\n<BODY><H4>501 Not supported</H4>\r\nMethod is not supported.\r\n</BODY></HTML>\r\n");
        return toReturn;
    }
    else if (strcmp(err, "403") == 0)
    {
        char *toReturn = (char *)calloc(1, sizeof(char));
        addStr(&toReturn, "HTTP/1.1 403 Forbidden\r\n");
        addStr(&toReturn, "Server: webserver /1.0\r\n");
        addStr(&toReturn, date);
        addStr(&toReturn, "Content-Type: text/html\r\n");
        addStr(&toReturn, "Content-Length: 111\r\n");
        addStr(&toReturn, "Connection: close\r\n\r\n");
        addStr(&toReturn, "<HTML><HEAD><TITLE>403 Forbidden</TITLE></HEAD>\r\n<BODY><H4>403 Forbidden</H4>\r\nAccess denied.\r\n</BODY></HTML>\r\n");
        return toReturn;
    }
    else if (strcmp(err, "404") == 0)
    {
        char *toReturn = (char *)calloc(1, sizeof(char));
        addStr(&toReturn, "HTTP/1.1 404 Not Found\r\n");
        addStr(&toReturn, "Server: webserver /1.0\r\n");
        addStr(&toReturn, date);
        addStr(&toReturn, "Content-Type: text/html\r\n");
        addStr(&toReturn, "Content-Length: 112\r\n");
        addStr(&toReturn, "Connection: close\r\n\r\n");
        addStr(&toReturn, "<HTML><HEAD><TITLE>404 Not Found</TITLE></HEAD>\r\n<BODY><H4>404 Not Found</H4>\r\nFile not found.\r\n</BODY></HTML>\r\n");
        return toReturn;
    }
    return "";
}

char *sendFile(char *path)
{
    struct stat fs;
    stat(path, &fs);
    char *toReturn = (char *)calloc(1, sizeof(char));
    char num[10];
    num[0] = 0;
    sprintf(num, "%ld", fs.st_size);
    char date[1000];
    date[0] = 0;
    time_t now = time(0);
    struct tm tm = *gmtime(&now);
    strftime(date, sizeof date, "%a, %d %b %Y %H:%M:%S %Z", &tm);
    char modify[1000];
    modify[0] = 0;
    strftime(modify, sizeof date, "%a, %d %b %Y %H:%M:%S %Z", gmtime(&fs.st_mtim.tv_sec));

    if (get_mime_type(path) == NULL)
    {
        return "";
    }
    addStr(&toReturn, "HTTP/1.1 200 OK\r\n");
    addStr(&toReturn, "Server: webserver/1.0\r\n");
    addStr(&toReturn, "Date: ");
    addStr(&toReturn, date);
    addStr(&toReturn, "\r\n");
    addStr(&toReturn, "Content-Type: ");
    addStr(&toReturn, get_mime_type(path));
    addStr(&toReturn, "\r\n");
    addStr(&toReturn, "Content-Length: ");
    addStr(&toReturn, num);
    addStr(&toReturn, "\r\n");
    addStr(&toReturn, "Last-Modified: ");
    addStr(&toReturn, modify);
    addStr(&toReturn, "\r\n");
    addStr(&toReturn, "Connection: close\r\n\r\n");
    return toReturn;
}

char *sendDir(char *path)
{

    char *html = (char *)calloc(1, sizeof(char));
    char *temp;
    addStr(&html, "<HTML>\r\n<HEAD><TITLE>Index of ");
    addStr(&html, path);
    addStr(&html, "</TITLE></HEAD>\r\n\r\n");
    addStr(&html, "<BODY>\r\n<H4>Index of ");
    addStr(&html, path);
    addStr(&html, "</H4>\r\n\r\n<table CELLSPACING=8>\r\n<tr><th>Name</th><th>Last Modified</th><th>Size</th></tr>\r\n\r\n\r\n");
    DIR *dir = opendir(path);
    struct dirent *dentry;
    while ((dentry = readdir(dir)) != NULL)
    {
        addStr(&html, "<tr>\r\n<td><A HREF=\"");
        addStr(&html, dentry->d_name);
        addStr(&html, "\">");
        addStr(&html, dentry->d_name);
        addStr(&html, "</A></td><td>");
        struct stat fs;
        temp = (char *)calloc(strlen(path) + 1, sizeof(char));
        addStr(&temp, path);
        addStr(&temp, dentry->d_name);
        stat(temp, &fs);
        if ((S_IROTH & fs.st_mode) != 0) // check permissions
        {
            char date[1000];
            date[0] = 0;
            strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&fs.st_mtim.tv_sec));
            addStr(&html, date);
            addStr(&html, "</td>\r\n");
            addStr(&html, "<td>");
            if (S_ISREG(fs.st_mode))
            {
                char num[10];
                num[0] = 0;
                sprintf(num, "%ld", fs.st_size);
                addStr(&html, num);
                addStr(&html, "</td>\r\n");
                addStr(&html, "</tr>\r\n\r\n");
            }
            else
            {
                addStr(&html, "</td>\r\n");
                addStr(&html, "</tr>\r\n\r\n");
            }
        }
        else
        {
            addStr(&html, "<td></td>\r\n<td></td>\r\n</tr>\r\n\r\n");
        }
    }
    addStr(&html, "</table>");
    char nowDate[1000];
    nowDate[0] = 0;
    time_t now = time(0);
    struct tm tm = *gmtime(&now);
    strftime(nowDate, sizeof nowDate, "%a, %d %b %Y %H:%M:%S %Z", &tm);
    strcat(nowDate, "\r\n");
    char conlen[10];
    conlen[0] = 0;
    sprintf(conlen, "%ld", strlen(html));
    conlen[strlen(conlen)] = 0;
    struct stat fs;
    stat(path, &fs);
    char date[1000];
    date[0] = 0;
    strftime(date, sizeof date, "%a, %d %b %Y %H:%M:%S %Z", gmtime(&fs.st_mtim.tv_sec));
    char *toReturn = (char *)calloc(1, sizeof(char));
    addStr(&toReturn, "HTTP/1.1 200 OK\r\n");
    addStr(&toReturn, "Server: webserver/1.0\r\n");
    addStr(&toReturn, nowDate);
    addStr(&toReturn, "Content-Type: text/html\r\n");
    addStr(&toReturn, "Content-Length: ");
    addStr(&toReturn, conlen);
    addStr(&toReturn, "\r\n");
    addStr(&toReturn, "Last-Modified:");
    addStr(&toReturn, date);
    addStr(&toReturn, "\r\n");
    addStr(&toReturn, "Connection: close\r\n\r\n");
    addStr(&toReturn, html);
    free(html);
    free(temp);
    free(dir);
    free(dentry);
    return toReturn;
}
int check(void *fd_ptr)
{
    int fd = 0;
    fd = *(int *)fd_ptr;
    int rc = 0;
    char *buffer = (char *)calloc(4000, sizeof(char));
    if (buffer == NULL)
    {
        write(fd, sendError("500"), strlen(sendError("500")));
        close(fd);
        return -1;
    }
    int counter = 0;
    int i = 0;
    int j = 0;
    int index = 0;
    int buff_len = 0;
    char *response = NULL;
    char temp[9];
    temp[0] = 0;
    rc = read(fd, buffer, BUFLEN); // read the request
    if (rc < 0)
    {
        response = sendError("500");
        write(fd, response, strlen(response));
        free(response);
        free(buffer);
        close(fd);
        return -1;
    }
    buff_len = strlen(buffer);
    char *request = (char *)calloc(buff_len + 1, sizeof(char));
    if (request == NULL)
    {
        response = sendError("500");
        write(fd, response, strlen(response));
        free(response);
        free(buffer);
        close(fd);
        return -1;
    }
    for (i = 0; i < buff_len; i++, j++) // Copy the first line of the request
    {
        if (buffer[i] == '\r' && buffer[i + 1] == '\n')
        {
            break;
        }
        else
            request[j] = buffer[i];
        while (buffer[i] == ' ' && buffer[i + 1] == ' ')
            i++;
    }
    if (strstr(request, "//") != NULL) // The path can not contain "//"
    {
        response = sendError("400");
        write(fd, response, strlen(response));
        free(response);
        free(request);
        free(buffer);
        close(fd);
        return -1;
    }
    for (i = 0; i < strlen(request); i++) // Counter the number of tokens
    {
        if (request[i] == ' ')
        {
            counter++;
            if (counter == 2)
                index = i + 1;
        }
    }
    if (counter != 2) // if there are not 3 tokens
    {
        response = sendError("400");
        write(fd, response, strlen(response));
        free(response);
        free(request);
        free(buffer);
        close(fd);
        return -1;
    }
    else if (counter == 2)
    {
        temp[8] = '\0';
        for (i = index, j = 0; j < 9; i++, j++) // Copy the last token
        {
            if (i < strlen(request))
                temp[j] = request[i];
        }
        if (strcmp(temp, "HTTP/1.1") != 0)
        {
            response = sendError("400");
            write(fd, response, strlen(response));
            free(response);
            free(request);
            free(buffer);
            close(fd);
            return -1;
        }
    }
    for (i = 0; i < 3; i++) // Copy
    {
        temp[i] = request[i];
    }
    temp[i] = '\0';
    if (strcmp(temp, "GET") != 0 || request[i] != ' ') // Check for the method
    {
        response = sendError("501");
        write(fd, response, strlen(response));
        free(response);
        free(request);
        free(buffer);
        close(fd);
        return -1;
    }
    char *path = (char *)calloc(index - 3, sizeof(char));
    strcpy(path, ".");
    for (i = 4, j = 1; i < index - 1; i++, j++) // Copy the path from the request
    {
        path[j] = request[i];
    }
    if (strcmp(path, ".") == 0)
    {
        if (request[4] == '/') // If the path is only "/"
        {
            response = sendDir("./"); // Send the content of the directory
            write(fd, response, strlen(response));
            free(response);
            free(request);
            free(path);
            free(buffer);
            close(fd);
            return -1;
        }
    }
    struct stat sb;
    int status = 0;
    status = stat(path, &sb);
    if (status == -1) // Check the return value of stat
    {
        response = sendError("404");
        write(fd, response, strlen(response));
        free(response);
        free(request);
        free(path);
        free(buffer);
        close(fd);
        return -1;
    }
    if (S_ISDIR(sb.st_mode)) // If the path is a directory
    {
        if (path[strlen(path) - 1] != '/') // If the path doesnt end with '/'
        {
            response = send302(path); // Send forbbiden error
            write(fd, response, strlen(response));
            free(response);
            free(request);
            free(path);
            free(buffer);
            close(fd);
            return -1;
        }
        char *lpath = (char *)calloc(strlen(path) + 11, sizeof(char));
        if (lpath == NULL)
        {
            response = sendError("500");
            write(fd, response, strlen(response));
            free(response);
            free(buffer);
            close(fd);
            return -1;
        }
        strcat(lpath, path);
        strcat(lpath, "index.html"); // Add "index.html to the path"
        int scounter = 0;
        for (i = 0; i < strlen(path); i++) // Count how much '/' in the path
        {
            if (path[i] == '/')
            {
                scounter++;
            }
        }
        char *copy = (char *)calloc(strlen(lpath) + 1, sizeof(char));
        if (copy == NULL)
        {
            response = sendError("500");
            write(fd, response, strlen(response));
            free(response);
            free(buffer);
            close(fd);
            return -1;
        }
        strcpy(copy, lpath);
        int check_status = checkPermission(path);
        if (check_status == -1)
        {
            response = sendError("403");
            write(fd, response, strlen(response));
            free(response);
            free(request);
            free(path);
            free(lpath);
            free(buffer);
            free(copy);
            close(fd);
            return -1;
        }
        else
        {
            if (access(copy, F_OK) != 0) // If there is no such file named index.html
            {
                response = sendDir(path); // Send the content of the directory
            }
            else // If there is a file named index.html
            {
                response = sendFile(copy); // Send the header of the file
            }
            write(fd, response, strlen(response));
            if (access(copy, F_OK) == 0)
            {
                FILE *f = fopen(copy, "r");
                unsigned char *buff = (unsigned char *)calloc(readdd, sizeof(unsigned char));
                int r = 0;
                while (buff != NULL)
                {
                    r = fread(buff, sizeof(unsigned char), readdd, f);
                    if (r == 0)
                        break;
                    write(fd, buff, readdd);
                    for (int k = 0; k < readdd; k++)
                    {
                        buffer[k] = 0;
                    }
                }
                free(buff);
            }
            write(fd, "\r\n", strlen("\r\n"));
            free(response);
            free(request);
            free(path);
            free(lpath);
            free(buffer);
            free(copy);
            close(fd);
            return 0;
        }
    }
    if (S_ISREG(sb.st_mode)) // If the path is a file
    {
        int check_status = checkPermission(path);
        if (check_status == 0) // If the file as read permissions
        {
            response = sendFile(path); // Send the header of the request
            write(fd, response, strlen(response));
            int d = open(path, O_RDONLY);
            char *buff = (char *)calloc(readdd, sizeof(char));
            int r = 0;
            while (1)
            {
                r = read(d, buff, readdd);
                if (r == 0)
                    break;
                write(fd, buff, readdd);
                for (int k = 0; k < readdd; k++)
                {
                    buff[k] = 0;
                }
            }
            free(buffer);
            free(buff);
            free(response);
            free(request);
            free(path);
            close(fd);
            return 0;
        }
    }
    // If the file is not a regular file
    response = sendError("403");
    write(fd, response, strlen(response));
    free(response);
    free(request);
    free(path);
    free(buffer);
    close(fd);
    return -1;
}

int main(int argc, char *argv[])
{
    int port = 0;
    int poolSize = 0;
    int max_request = 0;
    if (argc < 4)
    {
        perror("Usage: server <port> <pool-size> <max-number-of-request>\n");
        exit(1);
    }
    if (isNum(argv[1]) == -1)
    {
        perror("Usage: server <port> <pool-size> <max-number-of-request>\n");
        exit(1);
    }
    else
    {
        port = atoi(argv[1]);
    }

    if (isNum(argv[2]) == -1)
    {
        fprintf(stderr, "Usage: server <port> <pool-size> <max-number-of-request>\n");
        exit(1);
    }
    else if (atoi(argv[2]) > MAXT_IN_POOL)
    {
        fprintf(stderr, "Usage: server <port> <pool-size> <max-number-of-request>\n");
        exit(1);
    }
    else
    {
        poolSize = atoi(argv[2]);
    }
    if (isNum(argv[3]) == -1)
    {
        fprintf(stderr, "Usage: server <port> <pool-size> <max-number-of-request>\n");
        exit(1);
    }
    else
    {
        max_request = atoi(argv[3]);
    }
    int sockfd = 0;
    int newfd = 0;
    struct sockaddr_in serv_addr;
    struct sockaddr cli_addr;
    socklen_t clilen = 0;
    threadpool *t = create_threadpool(poolSize);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Error socket");
        exit(1);
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Error bind");
        exit(1);
    }
    if (listen(sockfd, 5) < 0)

    {
        perror("Error listen");
        exit(1);
    }

    for (int i = 0; i < max_request; i++)
    {
        newfd = accept(sockfd, &cli_addr, &clilen);
        if (newfd < 0)
        {
            perror("Error accept");
        }
        else
            dispatch(t, check, &newfd);
    }
    destroy_threadpool(t);
    close(sockfd);
}