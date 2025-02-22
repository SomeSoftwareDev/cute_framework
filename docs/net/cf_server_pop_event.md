[](../header.md ':include')

# cf_server_pop_event

Category: [net](/api_reference?id=net)  
GitHub: [cute_networking.h](https://github.com/RandyGaul/cute_framework/blob/master/include/cute_networking.h)  
---

Pops a [CF_ServerEvent](/net/cf_serverevent.md) off of the server, if available.

```cpp
bool cf_server_pop_event(CF_Server* server, CF_ServerEvent* event);
```

## Return Value

Returns true if an event was popped.

## Remarks

Server events notify of when a client connects/disconnects, or has sent a payload packet.
You must free the payload packets with [cf_server_free_packet](/net/cf_server_free_packet.md) when done.

## Related Pages

[CF_ServerEventType](/net/cf_servereventtype.md)  
[cf_server_event_type_to_string](/net/cf_server_event_type_to_string.md)  
[CF_ServerEvent](/net/cf_serverevent.md)  
[cf_server_send](/net/cf_server_send.md)  
[cf_server_update](/net/cf_server_update.md)  
