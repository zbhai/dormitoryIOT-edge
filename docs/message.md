# main
the message handle related to mqtt, message-dis, and dormitory modules.
the mqtt receive messages from device and forward it to message-dis.
the dormitory provide the message handler to message-dis.
the message-dis handle the message received from mqtt module, and the handler be provided by dormitory.

# feature
hold the message queue to store messages.
the object of fetch and push is message queue.
