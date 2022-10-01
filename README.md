# Gmail-File-System

1. The program creates a file system using FUSE.
2. It mounts the gmail onto a directory specified by the user.
3. Using IMAP, we are connecting to gmail.
4. Several commands are supported by our file system:
    mkdir, rmdir, mknod, ls, cd etc
5. The gmail is used to store the folders and files.


## A description of how this program works (i.e. its logic)
    1. This program takes input of mounting directory and root folder from user.
    2. It mounts the gmail on the mounting directory.
    3. It creates a folder root in gmail. All the files will be stored in this folder.
    4. The program uses FUSE to map terminal commands on gmail.
    5. Imap is used to fetch and append data in gmail.
    6. We can run various commands in terminal:
        - mkdir foldername: 
            Will create a new folder in gmail by the name foldername
        - cd foldername:
            Will cd into foldername
        - cd ..:
            Will cd back to parent folder
        - rmdir foldername: 
            Will remove the folder by name foldername
        - echo "Content of the file" >> filename.txt:
            This will create a new mail in 'root' directory with subject as filename.txt and body as 'content of the file'
        - ls: 
            Will list the folders present in 
        - vi filename.txt:
            Will open filename.txt in vim editor, where we can edit it.



## How to compile and run this program
    
    To compile the code:
    >>  gcc main.c -o main -lcurl `pkg-config fuse --cflags --libs`

    To run the code
    >> ./main -f [mount point] input.txt root

    NOTE: here [mount point] is the address of the directory to be used for mounting.
        input.txt is the file containing configuration inupts.
        root is the name of the root directory in gmail. All the files will be created inside [root] directory.


    Open another terminal in the mounted directory
    Use the commands as:

    mkdir foldername
    cd foldername
    cd ..
    rmdir foldername
    echo "Message" >> filename.txt
    ls
    vi filename.txt
