# admiral

admiral is a central message broker up 24/7 that allows different LIONS services to talk to each other using the LIONS Middleware Protocol.

I chose the message broker infrastructure instead of peer-to-peer communication due to my want for logging and shutting down LIONS all at once.

Configurations to admiral can be modified within `include/liblmp.h`. This consists of endpoints and their IDs, their IP addresses, and more.

admiral's functionality is subject to change without any notice. Updates will remain backwards compatible.
