# Repository

## Solution

For this challenge, the player is given a Java Spring application that acts as a repository for storing files. It comes with a default repository but also gives them the ability to add other types of repositories to their list.

One of the things they might notice right away is that various plugins are built as part of the docker image creation and placed in the ./plugins/storage directory. These plugins are later loaded dynamically, but they will have to examine the source code and discover that.

Because plugins are loaded dynamically from Jar files, it is possible to inject code into the app but we need to find a way to get a malicious file onto the server in a place where it can be executed. The upload function checks the file type of any files uploaded and does not allow us to directly upload something like a Jar file.

There is a copy function that allows us to download a file from a remote repo like HTTP to local. However, it also performs the same file type checking to prevent malicious files from being loaded. The key flaw in the process is that, while the download button on the frontend only passes a single file by default, the app is designed to download multiple files at once, and during this process there is a TOCTOU race condition between downloading to a temp folder, and checking the mime type of each file.

During the time while files are being downloaded, any files that are staged in a temp folder can still be accessed momentarily before they are checked, and we can take advantage of that by artificially slowing down a file in order to win the race.

To do this, we just need to initialize a remote HTTP repo that we control, and then ask to copy two files from it like ["../../../../app/plugins/storage/Malicious.jar","delay"] with the delay path causing the remote repo to sleep for 5 seconds. This takes advantage of a path traversal vulnerability that is also present in the copy function to place a malicious jar file in the plugins folder, and then delay checking it for invalid mime types.

While the remote repo is sleeping, we ask the app to initialize a new repo, which loads the malicious jar file from the plugin folder, and then ask it for the flag. As long as we are able to do this while the delay is still happening, then we can win the race condition and exfiltrate the flag before the evil jar is deleted. The Jar itself just needs to be built in the proper way to be loaded and call functions. For my solution code, I just used the legitimate LocalStorage plugin and modified it to return the flag file at /flag.txt when requested.

```java
public Map<String, Object> getFileContent(String filename) throws IOException {

    Map<String, Object> response = new HashMap<>();

    File file = new File("/flag.txt");
    byte[] data = Files.readAllBytes(file.toPath());
    response.put("mime", "text/plain");
    response.put("content", data);
    response.put("success", true);

    return response;
}
```