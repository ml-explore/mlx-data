# bxzstr documentation
This file details how to implement support for new compression types,
particularly those that do not have an API that is (almost) drop-in
replacement for zlib.

## Adding support for new compression types
### Creating a stream wrapper
A stream wrapper file forms the core of the compression type support
in bxzstr. Most types _should_ be implementable by simply defining a
`compression_type_stream_wrapper` class that implements the functions
from the abstract `stream_wrapper` defined in
[include/stream_wrapper.hpp](/include/stream_wrapper.hpp). These
functions are used by the bxzstr ostreambuf and istreambuf classes to
manipulate the compressed stream without touching the implementation
of the compression algorithm itself.

Each stream wrapper should implement functionality for both
compression and decompression operations. The difference between them
is determined by an internal state variable.

### Functions defined in stream\_wrapper
These are the functions that every stream\_wrapper class needs to implement:
```
    stream_wrapper(const bool _isInput, const int _level, const int _flags);
    virtual ~stream_wrapper() = default;
    virtual int decompress(const int _flags = 0) =0;
    virtual int compress(const int _flags = 0) =0;
    virtual bool stream_end() const =0;
    virtual bool done() const =0;

    virtual const uint8_t* next_in() const =0;
    virtual long avail_in() const =0;
    virtual uint8_t* next_out() const =0;
    virtual long avail_out() const =0;

    virtual void set_next_in(const unsigned char* in) =0;
    virtual void set_avail_in(const long in) =0;
    virtual void set_next_out(const uint8_t* in) =0;
    virtual void set_avail_out(const long in) =0;
```

The purpose of each function in the bxzstr write and read operations
will be covered below.

#### Constructor
##### stream\_wrapper(const bool \_isInput, const int \_level, const int \_flags))
Each stream\_wrapper should have a constructor that accepts the following arguments:
- \_isInput: define whether we are compressing or decompressing a stream.
- \_level: compression level for the algorithm.
- \_flags: optional integer parameter.

The the \_level and/or \_flags arguments may default to 0 if they are
not required.

The constructor should initialize the stream as the correct
compressing or decompressing object with the supplied arguments. If
there are problems in initializing the stream, the constructor should
throw an error (more about error). If the compression API
provides functions for initializing the stream and checking for
errors, these should be used.

#### Destructor
##### ~stream_wrapper();
The destructor should at minimum flush (end) the stream and deallocate
all memory that may have been allocated using the C-style
functions. If the compression API provides a function for ending the
stream, this should be used.

#### Compression and decompression operations
##### int decompress(const int \_flags = 0)
Implements the decompression operations by calling the decompression
function from the compression API. If the API does not update the
state of the stream, the updates should be performed here after the
call. See
[include/zstd\_stream\_wrapper.hpp](/include/zstd_stream_wrapper.hpp)
for more details on how to implement this. Zlib-like APIs track the
stream status internally. The argument \_flags may be used to pass
parameters to the decompression call.

##### int compress(const int \_flags)
As above, but implements the compression operations. The \_flags
argument should be used to signify that the current chunk is the last
chunk to compress, and the appropriate function for flushing the
compressed stream should be called.

#### Stream status getters
##### bool stream\_end() const
Returns 1 if the internal stream state has reached its end, 0
otherwise.

##### bool done() const
Returns 1 if stream\_end() is true _or_ we have reached an error
message that signifies the end of the compression/decompression but
does not necessarily cause the program to abort.

##### const uint8\_t* next\_in()
Returns a pointer to the current position in the inbuffer.

##### long avail\_in() const
Returns the size of the current inbuffer.

##### uint8\_t* next\_out()
Returns a pointer to the current position in the outbuffer.

##### long avail\_out() const
Returns the size of the current outbuffer.

#### Stream status setters
##### void set\_next\_in(const unsigned char* in)
Sets the current position in the inbuffer to _in_.

##### void set\_avail\_in(const long in)
Sets the size of the current inbuffer to _in_.

##### void set\_next\_out(const uint8\_t* in)
Sets the current position in the outbuffer to _in_.

##### void set\_avail\_out(const long in)
Sets the size of the current outbuffer to _in_.
