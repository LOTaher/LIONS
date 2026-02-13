# archimedes

archimedes is the LIONS port forwarding daemon that keeps self-hosted services port forwarded via SSH up 24/7.

An `uber.json` file is required that follows the following format:

```json
{
    "db": [
        "nuke",
        "5432",
        "5050"
    ],
    "web": [
        "nuke",
        "80",
        "8080"
    ],
    "admiral": [
        "inferno",
        "22",
        "1404"
    ]
}
```

Where each service within the file must include the following array elements in order:
- Name of client as labeled within your `.ssh/config`
- The port the service is listening on **remotely**
- The port you'd like the service to be forwarded to  **locally**

**Usage**: `./archimedes &`

This is a program that is designed for me, so archimedes shuts down all SSH processes when it exits.
Feel free to modify the code to handle program exits differently.

archimedes' functionality is subject to change without any notice. Updates will remain backwards compatible.
