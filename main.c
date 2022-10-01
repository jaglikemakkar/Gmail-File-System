// Payload source

#define FUSE_USE_VERSION 30

// Importing Libraries
#include <fuse.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <curl/curl.h>

#define FROM_ADDR "<2019csb1092@iitrpr.ac.in>"
#define TO_ADDR "<2019csb1092@iitrpr.ac.in>"
#define CC_ADDR "<2019csb1092@iitrpr.ac.in>"

#define FROM_MAIL "Sender " FROM_ADDR
#define TO_MAIL "Receiver " TO_ADDR
#define CC_MAIL "CCer" CC_ADDR

// Contains list of folders created
char dir_list[256][256];
int curr_dir_idx = -1;

// Contains contents of files
char files_list[256][256];
int curr_file_idx = -1;

char files_content[256][256];
int curr_file_content_idx = -1;
char *payload_text;

char SERVER_ADDR[256];
char PORT_NUMBER[256];
char USERNAME[256];
char PASSWORD[256];

char *root;

void add_dir(const char *dir_name)
{
    CURL *curl;
    CURLcode res = CURLE_OK;

    curl = curl_easy_init();
    if (curl)
    {
        printf("Creating folder..\n");
        /* Set username and password */
        curl_easy_setopt(curl, CURLOPT_USERNAME, USERNAME);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, PASSWORD);

        /* This is just the server URL */
        char imap_addr[100];
        memset(imap_addr, '\0', sizeof(imap_addr));
        strcat(imap_addr, SERVER_ADDR);
        strcat(imap_addr, ":");
        strcat(imap_addr, PORT_NUMBER);
        strcat(imap_addr, "/");
        curl_easy_setopt(curl, CURLOPT_URL, imap_addr);

        /* Set the CREATE command specifying the new folder name */
        char create_commad[200];
        memset(create_commad, '\0', sizeof(create_commad));
        strcat(create_commad, "CREATE ");
        strcat(create_commad, dir_name);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, create_commad);

        /* Perform the custom request */
        res = curl_easy_perform(curl);

        /* Check for errors */
        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));

        /* Always cleanup */
        curl_easy_cleanup(curl);
    }
    curr_dir_idx += 1;
    strcpy(dir_list[curr_dir_idx], dir_name);
}

int is_dir(const char *path)
{
    path++; // Eliminating "/" in the path

    for (int i = 0; i < curr_dir_idx + 1; i++)
    {
        if (strcmp(path, dir_list[i]) == 0)
        {
            return 1;
        }
    }
    return 0;
}

struct upload_status
{
    size_t bytes_read;
};

static size_t payload_source(char *ptr, size_t size, size_t nmemb, void *userp)
{
    struct upload_status *upload_ctx = (struct upload_status *)userp;
    const char *data;
    size_t room = size * nmemb;

    if ((size == 0) || (nmemb == 0) || ((size * nmemb) < 1))
    {
        return 0;
    }

    data = &payload_text[upload_ctx->bytes_read];

    if (*data)
    {
        size_t len = strlen(data);
        if (room < len)
            len = room;
        memcpy(ptr, data, len);
        upload_ctx->bytes_read += len;

        return len;
    }
    return 0;
}

int send_mail(void)
{
    CURL *curl;
    CURLcode res = CURLE_OK;

    curl = curl_easy_init();
    if (curl)
    {
        printf("Sending mail...\n");
        long infilesize;
        struct upload_status upload_ctx = {0};

        /* Set username and password */
        curl_easy_setopt(curl, CURLOPT_USERNAME, USERNAME);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, PASSWORD);

        /* This will create a new message 100. Note that you should perform an
        * EXAMINE command to obtain the UID of the next message to create and a
        * SELECT to ensure you are creating the message in the OUTBOX. */
        char imap_addr[100];
        memset(imap_addr, '\0', sizeof(imap_addr));
        strcat(imap_addr, SERVER_ADDR);
        strcat(imap_addr, "/");
        strcat(imap_addr, root);
        curl_easy_setopt(curl, CURLOPT_URL, imap_addr);

        /* In this case, we are using a callback function to specify the data. You
     * could just use the CURLOPT_READDATA option to specify a FILE pointer to
     * read from. */
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
        curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

        infilesize = strlen(payload_text);
        curl_easy_setopt(curl, CURLOPT_INFILESIZE, infilesize);

        /* Perform the append */
        res = curl_easy_perform(curl);

        /* Check for errors */
        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));

        /* Always cleanup */
        curl_easy_cleanup(curl);
    }

    return (int)res;
}

void create_new_email(const char *filename, const char *content)
{
    // Creating the message string
    printf("Creating Mail..\n");
    char *temp = strrchr(filename, '.');
    char last = temp[strlen(temp) - 1];
    temp[strlen(temp) - 1] = '\0';
    if (strcmp(temp + 1, "sw") == 0)
        return;
    char message[2000];
    temp[strlen(temp)] = last;
    memset(message, '\0', sizeof(message));
    strcat(message, "Date: Mon, 29 Nov 2010 21:54:29 +1100\r\n");
    strcat(message, "To: " TO_ADDR "(To User)"
                    "\r\n");
    strcat(message, "From: " FROM_ADDR "(From User)"
                    "\r\n");
    strcat(message, "Cc: " FROM_ADDR "(CC User)"
                    "\r\n");
    strcat(message, "Message-ID: <dcd7cb36-11db-487a-9f3a-e652a9458efd@rfcpedant.example.org>"
                    "\r\n");
    strcat(message, "Subject: ");
    strcat(message, filename);
    strcat(message, "\r\n\r\n");
    strcat(message, content);

    // Storing the message in payload_text
    payload_text = message;

    // Sending mail
    send_mail();
    printf("End of create new email file with filename: %s\n", filename);
}

// Function to add file
void add_file(const char *filename)
{
    printf("Adding a file : %s\n", filename);
    const char *content = "";

    // Creating new email
    create_new_email(filename, content);

    curr_file_idx++;
    strcpy(files_list[curr_file_idx], filename);

    curr_file_content_idx++;
    strcpy(files_content[curr_file_content_idx], "");
}

//Checking if file is present
int is_file(const char *path)
{
    path++; // Eliminating "/" in the path

    for (int curr_idx = 0; curr_idx <= curr_file_idx; curr_idx++)
        if (strcmp(path, files_list[curr_idx]) == 0)
            return 1;

    return 0;
}

int get_file_index(const char *path)
{
    path++; // Eliminating "/" in the path

    for (int curr_idx = 0; curr_idx <= curr_file_idx; curr_idx++)
        if (strcmp(path, files_list[curr_idx]) == 0)
            return curr_idx;

    return -1;
}

void write_to_file(const char *path, const char *new_content)
{
    int file_idx = get_file_index(path);

    if (file_idx == -1) // No such file
        return;
    path++;
    create_new_email(path, new_content);
    strcpy(files_content[file_idx], new_content);
}

static int do_getattr(const char *path, struct stat *st)
{
    st->st_uid = getuid();     // The owner of the file/directory is the user who mounted the filesystem
    st->st_gid = getgid();     // The group of the file/directory is the same as the group of the user who mounted the filesystem
    st->st_atime = time(NULL); // The last "a"ccess of the file/directory is right now
    st->st_mtime = time(NULL); // The last "m"odification of the file/directory is right now

    if (strcmp(path, "/") == 0 || is_dir(path) == 1)
    {
        st->st_mode = S_IFDIR | 0755;
        st->st_nlink = 2; // Why "two" hardlinks instead of "one"? The answer is here: http://unix.stackexchange.com/a/101536
    }
    else if (is_file(path) == 1)
    {
        st->st_mode = S_IFREG | 0644;
        st->st_nlink = 1;
        st->st_size = 1024;
    }
    else
    {
        return -ENOENT;
    }

    return 0;
}

static int do_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
    filler(buffer, ".", NULL, 0);  // Current Directory
    filler(buffer, "..", NULL, 0); // Parent Directory

    if (strcmp(path, "/") == 0) // If the user is trying to show the files/directories of the root directory show the following
    {
        for (int curr_idx = 0; curr_idx <= curr_dir_idx; curr_idx += 1){
            filler(buffer, dir_list[curr_idx], NULL, 0);
        }

        for (int curr_idx = 0; curr_idx <= curr_file_idx; curr_idx = curr_idx+1){
            filler(buffer, files_list[curr_idx], NULL, 0);
        }
    }

    return 0;
}

static int do_read(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi)
{
    int file_idx = get_file_index(path);
    if (file_idx == -1)
        return -1;

    char *content = files_content[file_idx];
    memcpy(buffer, content + offset, size);
    return strlen(content) - offset;
}

static int do_mkdir(const char *path, mode_t mode)
{
    path++;
    add_dir(path);
    return 0;
}

static int do_mknod(const char *path, mode_t mode, dev_t rdev)
{
    path++;
    add_file(path);
    return 0;
}

static int do_write(const char *path, const char *buffer, size_t size, off_t offset, struct fuse_file_info *info)
{
    write_to_file(path, buffer);
    return size;
}

void rem_dir(const char *path)
{
    CURL *curl;
    CURLcode res = CURLE_OK;

    curl = curl_easy_init();
    if (curl)
    {
        /* Set username and password */
        curl_easy_setopt(curl, CURLOPT_USERNAME, USERNAME);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, PASSWORD);

        /* This is just the server URL */
        char imap_addr[100];
        memset(imap_addr, '\0', sizeof(imap_addr));
        strcat(imap_addr, SERVER_ADDR);
        strcat(imap_addr, ":");
        strcat(imap_addr, PORT_NUMBER);
        strcat(imap_addr, "/");
        curl_easy_setopt(curl, CURLOPT_URL, imap_addr);

        char temp[200];
        memset(temp, '\0', sizeof(temp));
        strcat(temp, "DELETE ");
        strcat(temp, path);

        /* Set the DELETE command specifying the existing folder */
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, temp);

        /* Perform the custom request */
        res = curl_easy_perform(curl);

        /* Check for errors */
        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));

        /* Always cleanup */
        curl_easy_cleanup(curl);
    }
}

static int do_rmdir(const char *path)
{
    path++;

    if (strcmp(path, root) == 0)
    {
        printf("Cannot delete ROOT folder\n");
        return 0;
    }
    int ind = -1;
    for (int i = 0; i < curr_dir_idx + 1; i = i+1)
    {
        if (strcmp(path, dir_list[i]) == 0)
        {
            memset(dir_list[i], '\0', sizeof(dir_list[i]));
            ind = i;
            break;
        }
    }

    if (ind == -1)
    {
        return 0;
    }

    for (int i = ind; i < curr_dir_idx; i = i+1)
    {
        strcpy(dir_list[i], dir_list[i + 1]);
    }
    curr_dir_idx--;

    rem_dir(path);

    return 0;
}

static int do_rmfile(const char *path)
{
    path++;
    if (isdigit(path[0])){
        printf("Cannot Delete Append-only File\n");
        return 0;
    }
    int ind = -1;
    for (int curr_idx = 0; curr_idx <= curr_file_idx; curr_idx++)
    {
        if (strcmp(path, files_list[curr_idx]) == 0)
        {
            memset(files_list[curr_idx], '\0', sizeof(files_list[curr_idx]));
            ind = curr_idx;

            memset(files_content[curr_idx], '\0', sizeof(files_content[curr_idx]));
            break;
        }
    }

    if (ind == -1)
    {
        return 0;
    }

    for (int i = ind; i < curr_file_idx; i++)
    {
        strcpy(files_list[i], files_list[i + 1]);
        strcpy(files_content[i], files_list[i + 1]);
    }
    curr_file_idx--;
    curr_file_content_idx--;
    return 0;
}

static struct fuse_operations operations = {
    .getattr = do_getattr,
    .readdir = do_readdir,
    .read = do_read,
    .mkdir = do_mkdir,
    .mknod = do_mknod,
    .write = do_write,
    .rmdir = do_rmdir,
    .unlink = do_rmfile,
    // .unlink		= do_unlink,
};

struct string
{
    char *ptr;
    size_t len;
};

void init_string(struct string *s)
{
    s->len = 0;
    s->ptr = malloc(s->len + 1);
    if (s->ptr == NULL)
    {
        fprintf(stderr, "malloc() failed\n");
        exit(EXIT_FAILURE);
    }
    s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s)
{
    size_t new_len = s->len + size * nmemb;
    s->ptr = realloc(s->ptr, new_len + 1);
    if (s->ptr == NULL)
    {
        fprintf(stderr, "realloc() failed\n");
        exit(EXIT_FAILURE);
    }
    memcpy(s->ptr + s->len, ptr, size * nmemb);
    s->ptr[new_len] = '\0';
    s->len = new_len;

    return size * nmemb;
}

void fetch_data()
{
    CURL *curl;
    CURLcode res = CURLE_OK;

    curl = curl_easy_init();
    if (curl)
    {
        struct string s;
        init_string(&s);

        curl_easy_setopt(curl, CURLOPT_USERNAME, USERNAME);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, PASSWORD);

        /* This is just the server URL */
        char imap_addr[100];
        memset(imap_addr, '\0', sizeof(imap_addr));
        strcat(imap_addr, SERVER_ADDR);
        strcat(imap_addr, ":");
        strcat(imap_addr, PORT_NUMBER);
        strcat(imap_addr, "/");
        curl_easy_setopt(curl, CURLOPT_URL, imap_addr);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
        res = curl_easy_perform(curl);

        char *p = strtok(s.ptr, "\n");
        while (p != NULL)
        {
            char *temp = p;
            char *last = strrchr(temp, '/');
            if (last != NULL)
            {
                while (!isalnum(*last))
                    last++;
                char *foldername = last;
                foldername[strlen(foldername) - 2] = '\0';
                if (strcmp(foldername, "Gmail]") != 0)
                {
                    curr_dir_idx++;
                    strcpy(dir_list[curr_dir_idx], foldername);
                }
            }
            p = strtok(NULL, "\n");
        }
        free(s.ptr);
        /* Check for errors */
        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
        }
        /* always cleanup */
        curl_easy_cleanup(curl);
    }
}

int main(int argc, char *argv[])
{
    if (argc < 5){
        printf("USAGE: %s -f [mount point] [configure file path] [root]\n", argv[0]);
        exit(0);
    }
    FILE *file_ptr;
    char curr;

    // Reading root directory
    root = argv[4];
    argc -= 1;

    // Opening file
    file_ptr = fopen(argv[3], "r");
    if (file_ptr == NULL)
    {
        printf("File path incorrect \n");
        exit(0);
    }

    curr = fgetc(file_ptr);

    // Reading contents of file
    char temp[256];
    int temp_idx = 0;
    int cnt = 0;
    while (1)
    {
        if (curr == EOF)
            break;

        if (curr != '\n')
        {
            temp[temp_idx] = curr;
            temp_idx += 1;
        }
        else
        {
            if (cnt == 0)
                strcpy(SERVER_ADDR, temp); // Reading server address

            else if (cnt == 1)
                strcpy(PORT_NUMBER, temp); // Reading Port Number

            else if (cnt == 2)
                strcpy(USERNAME, temp); // Reading Username

            else
            {
                strcpy(PASSWORD, temp); // Reading Password
                break;
            }
            memset(temp, '\0', sizeof(temp));
            temp_idx = 0;
            cnt = cnt + 1;
        }
        curr = fgetc(file_ptr);
    }
    argc = argc - 1;

    fetch_data(); // Fetching data from gmail

    add_dir(root); // Adding ROOT directory

    return fuse_main(argc, argv, &operations, NULL);
}