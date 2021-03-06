//===-- File.h --------------------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef liblldb_File_h_
#define liblldb_File_h_

// C Includes
// C++ Includes
#include <stdarg.h>
#include <stdio.h>
#include <sys/types.h>

// Other libraries and framework includes
// Project includes
#include "lldb/lldb-private.h"
#include "lldb/Host/IOObject.h"

namespace lldb_private {

//----------------------------------------------------------------------
/// @class File File.h "lldb/Host/File.h"
/// @brief A file class.
///
/// A file class that divides abstracts the LLDB core from host file
/// functionality.
//----------------------------------------------------------------------
class File : public IOObject
{
public:
    static int kInvalidDescriptor;
    static FILE * kInvalidStream;

    enum OpenOptions
    {
        eOpenOptionRead                 = (1u << 0),    // Open file for reading
        eOpenOptionWrite                = (1u << 1),    // Open file for writing
        eOpenOptionAppend               = (1u << 2),    // Don't truncate file when opening, append to end of file
        eOpenOptionTruncate             = (1u << 3),    // Truncate file when opening
        eOpenOptionNonBlocking          = (1u << 4),    // File reads
        eOpenOptionCanCreate            = (1u << 5),    // Create file if doesn't already exist
        eOpenOptionCanCreateNewOnly     = (1u << 6),    // Can create file only if it doesn't already exist
        eOpenoptionDontFollowSymlinks   = (1u << 7),
        eOpenOptionCloseOnExec          = (1u << 8)     // Close the file when executing a new process
    };
    
    static mode_t
    ConvertOpenOptionsForPOSIXOpen (uint32_t open_options);
    
    File() : 
        IOObject(eFDTypeFile, false),
        m_descriptor (kInvalidDescriptor),
        m_stream (kInvalidStream),
        m_options (0),
        m_own_stream (false),
        m_is_interactive (eLazyBoolCalculate),
        m_is_real_terminal (eLazyBoolCalculate),
        m_supports_colors (eLazyBoolCalculate)
    {
    }
    
    File (FILE *fh, bool transfer_ownership) :
        IOObject(eFDTypeFile, false),
        m_descriptor (kInvalidDescriptor),
        m_stream (fh),
        m_options (0),
        m_own_stream (transfer_ownership),
        m_is_interactive (eLazyBoolCalculate),
        m_is_real_terminal (eLazyBoolCalculate),
        m_supports_colors (eLazyBoolCalculate)
    {
    }

    //------------------------------------------------------------------
    /// Constructor with path.
    ///
    /// Takes a path to a file which can be just a filename, or a full
    /// path. If \a path is not nullptr or empty, this function will call
    /// File::Open (const char *path, uint32_t options, uint32_t permissions).
    ///
    /// @param[in] path
    ///     The full or partial path to a file.
    ///
    /// @param[in] options
    ///     Options to use when opening (see File::OpenOptions)
    ///
    /// @param[in] permissions
    ///     Options to use when opening (see File::Permissions)
    ///
    /// @see File::Open (const char *path, uint32_t options, uint32_t permissions)
    //------------------------------------------------------------------
    File (const char *path,
          uint32_t options,
          uint32_t permissions = lldb::eFilePermissionsFileDefault);

    //------------------------------------------------------------------
    /// Constructor with FileSpec.
    ///
    /// Takes a FileSpec pointing to a file which can be just a filename, or a full
    /// path. If \a path is not nullptr or empty, this function will call
    /// File::Open (const char *path, uint32_t options, uint32_t permissions).
    ///
    /// @param[in] filespec
    ///     The FileSpec for this file.
    ///
    /// @param[in] options
    ///     Options to use when opening (see File::OpenOptions)
    ///
    /// @param[in] permissions
    ///     Options to use when opening (see File::Permissions)
    ///
    /// @see File::Open (const char *path, uint32_t options, uint32_t permissions)
    //------------------------------------------------------------------
    File (const FileSpec& filespec,
          uint32_t options,
          uint32_t permissions = lldb::eFilePermissionsFileDefault);
    
    File (int fd, bool transfer_ownership) :
        IOObject(eFDTypeFile, transfer_ownership),
        m_descriptor (fd),
        m_stream (kInvalidStream),
        m_options (0),
        m_own_stream (false),
        m_is_interactive (eLazyBoolCalculate),
        m_is_real_terminal (eLazyBoolCalculate),
        m_supports_colors (eLazyBoolCalculate)
    {
    }

    //------------------------------------------------------------------
    /// Destructor.
    ///
    /// The destructor is virtual in case this class is subclassed.
    //------------------------------------------------------------------
    ~File() override;

    bool
    IsValid() const override
    {
        return DescriptorIsValid() || StreamIsValid();
    }

    //------------------------------------------------------------------
    /// Convert to pointer operator.
    ///
    /// This allows code to check a File object to see if it
    /// contains anything valid using code such as:
    ///
    /// @code
    /// File file(...);
    /// if (file)
    /// { ...
    /// @endcode
    ///
    /// @return
    ///     A pointer to this object if either the directory or filename
    ///     is valid, nullptr otherwise.
    //------------------------------------------------------------------
    operator
    bool () const
    {
        return DescriptorIsValid() || StreamIsValid();
    }

    //------------------------------------------------------------------
    /// Logical NOT operator.
    ///
    /// This allows code to check a File object to see if it is
    /// invalid using code such as:
    ///
    /// @code
    /// File file(...);
    /// if (!file)
    /// { ...
    /// @endcode
    ///
    /// @return
    ///     Returns \b true if the object has an empty directory and
    ///     filename, \b false otherwise.
    //------------------------------------------------------------------
    bool
    operator! () const
    {
        return !DescriptorIsValid() && !StreamIsValid();
    }

    //------------------------------------------------------------------
    /// Get the file spec for this file.
    ///
    /// @return
    ///     A reference to the file specification object.
    //------------------------------------------------------------------
    Error
    GetFileSpec (FileSpec &file_spec) const;
    
    //------------------------------------------------------------------
    /// Open a file for read/writing with the specified options.
    ///
    /// Takes a path to a file which can be just a filename, or a full
    /// path.
    ///
    /// @param[in] path
    ///     The full or partial path to a file.
    ///
    /// @param[in] options
    ///     Options to use when opening (see File::OpenOptions)
    ///
    /// @param[in] permissions
    ///     Options to use when opening (see File::Permissions)
    //------------------------------------------------------------------
    Error
    Open (const char *path,
          uint32_t options,
          uint32_t permissions = lldb::eFilePermissionsFileDefault);

    Error
    Close() override;
    
    void
    Clear ();
    
    int
    GetDescriptor() const;

    WaitableHandle
    GetWaitableHandle() override;

    void
    SetDescriptor(int fd, bool transfer_ownership);

    FILE *
    GetStream ();

    void
    SetStream (FILE *fh, bool transfer_ownership);

    //------------------------------------------------------------------
    /// Read bytes from a file from the current file position.
    ///
    /// NOTE: This function is NOT thread safe. Use the read function
    /// that takes an "off_t &offset" to ensure correct operation in
    /// multi-threaded environments.
    ///
    /// @param[in] buf
    ///     A buffer where to put the bytes that are read.
    ///
    /// @param[in,out] num_bytes
    ///     The number of bytes to read form the current file position
    ///     which gets modified with the number of bytes that were read.
    ///
    /// @return
    ///     An error object that indicates success or the reason for 
    ///     failure.
    //------------------------------------------------------------------
    Error
    Read(void *buf, size_t &num_bytes) override;
          
    //------------------------------------------------------------------
    /// Write bytes to a file at the current file position.
    ///
    /// NOTE: This function is NOT thread safe. Use the write function
    /// that takes an "off_t &offset" to ensure correct operation in
    /// multi-threaded environments.
    ///
    /// @param[in] buf
    ///     A buffer where to put the bytes that are read.
    ///
    /// @param[in,out] num_bytes
    ///     The number of bytes to write to the current file position
    ///     which gets modified with the number of bytes that were 
    ///     written.
    ///
    /// @return
    ///     An error object that indicates success or the reason for 
    ///     failure.
    //------------------------------------------------------------------
    Error
    Write(const void *buf, size_t &num_bytes) override;

    //------------------------------------------------------------------
    /// Seek to an offset relative to the beginning of the file.
    ///
    /// NOTE: This function is NOT thread safe, other threads that 
    /// access this object might also change the current file position.
    /// For thread safe reads and writes see the following functions:
    /// @see File::Read (void *, size_t, off_t &)
    /// @see File::Write (const void *, size_t, off_t &)
    ///
    /// @param[in] offset
    ///     The offset to seek to within the file relative to the 
    ///     beginning of the file.
    ///
    /// @param[in] error_ptr
    ///     A pointer to a lldb_private::Error object that will be
    ///     filled in if non-nullptr.
    ///
    /// @return
    ///     The resulting seek offset, or -1 on error.
    //------------------------------------------------------------------
    off_t
    SeekFromStart(off_t offset, Error *error_ptr = nullptr);
    
    //------------------------------------------------------------------
    /// Seek to an offset relative to the current file position.
    ///
    /// NOTE: This function is NOT thread safe, other threads that 
    /// access this object might also change the current file position.
    /// For thread safe reads and writes see the following functions:
    /// @see File::Read (void *, size_t, off_t &)
    /// @see File::Write (const void *, size_t, off_t &)
    ///
    /// @param[in] offset
    ///     The offset to seek to within the file relative to the 
    ///     current file position.
    ///
    /// @param[in] error_ptr
    ///     A pointer to a lldb_private::Error object that will be
    ///     filled in if non-nullptr.
    ///
    /// @return
    ///     The resulting seek offset, or -1 on error.
    //------------------------------------------------------------------
    off_t
    SeekFromCurrent(off_t offset, Error *error_ptr = nullptr);
    
    //------------------------------------------------------------------
    /// Seek to an offset relative to the end of the file.
    ///
    /// NOTE: This function is NOT thread safe, other threads that 
    /// access this object might also change the current file position.
    /// For thread safe reads and writes see the following functions:
    /// @see File::Read (void *, size_t, off_t &)
    /// @see File::Write (const void *, size_t, off_t &)
    ///
    /// @param[in,out] offset
    ///     The offset to seek to within the file relative to the 
    ///     end of the file which gets filled in with the resulting
    ///     absolute file offset.
    ///
    /// @param[in] error_ptr
    ///     A pointer to a lldb_private::Error object that will be
    ///     filled in if non-nullptr.
    ///
    /// @return
    ///     The resulting seek offset, or -1 on error.
    //------------------------------------------------------------------
    off_t
    SeekFromEnd(off_t offset, Error *error_ptr = nullptr);

    //------------------------------------------------------------------
    /// Read bytes from a file from the specified file offset.
    ///
    /// NOTE: This function is thread safe in that clients manager their
    /// own file position markers and reads on other threads won't mess
    /// up the current read.
    ///
    /// @param[in] dst
    ///     A buffer where to put the bytes that are read.
    ///
    /// @param[in,out] num_bytes
    ///     The number of bytes to read form the current file position
    ///     which gets modified with the number of bytes that were read.
    ///
    /// @param[in,out] offset
    ///     The offset within the file from which to read \a num_bytes
    ///     bytes. This offset gets incremented by the number of bytes
    ///     that were read.
    ///
    /// @return
    ///     An error object that indicates success or the reason for 
    ///     failure.
    //------------------------------------------------------------------
    Error
    Read (void *dst, size_t &num_bytes, off_t &offset);
    
    //------------------------------------------------------------------
    /// Read bytes from a file from the specified file offset.
    ///
    /// NOTE: This function is thread safe in that clients manager their
    /// own file position markers and reads on other threads won't mess
    /// up the current read.
    ///
    /// @param[in,out] num_bytes
    ///     The number of bytes to read form the current file position
    ///     which gets modified with the number of bytes that were read.
    ///
    /// @param[in,out] offset
    ///     The offset within the file from which to read \a num_bytes
    ///     bytes. This offset gets incremented by the number of bytes
    ///     that were read.
    ///
    /// @param[in] null_terminate
    ///     Ensure that the data that is read is terminated with a NULL
    ///     character so that the data can be used as a C string.
    ///
    /// @param[out] data_buffer_sp
    ///     A data buffer to create and fill in that will contain any
    ///     data that is read from the file. This buffer will be reset
    ///     if an error occurs.
    ///
    /// @return
    ///     An error object that indicates success or the reason for 
    ///     failure.
    //------------------------------------------------------------------
    Error
    Read (size_t &num_bytes,
          off_t &offset,
          bool null_terminate,
          lldb::DataBufferSP &data_buffer_sp);

    //------------------------------------------------------------------
    /// Write bytes to a file at the specified file offset.
    ///
    /// NOTE: This function is thread safe in that clients manager their
    /// own file position markers, though clients will need to implement
    /// their own locking externally to avoid multiple people writing
    /// to the file at the same time.
    ///
    /// @param[in] src
    ///     A buffer containing the bytes to write.
    ///
    /// @param[in,out] num_bytes
    ///     The number of bytes to write to the file at offset \a offset.
    ///     \a num_bytes gets modified with the number of bytes that 
    ///     were read.
    ///
    /// @param[in,out] offset
    ///     The offset within the file at which to write \a num_bytes
    ///     bytes. This offset gets incremented by the number of bytes
    ///     that were written.
    ///
    /// @return
    ///     An error object that indicates success or the reason for 
    ///     failure.
    //------------------------------------------------------------------
    Error
    Write (const void *src, size_t &num_bytes, off_t &offset);

    //------------------------------------------------------------------
    /// Flush the current stream
    ///
    /// @return
    ///     An error object that indicates success or the reason for 
    ///     failure.
    //------------------------------------------------------------------
    Error
    Flush ();
    
    //------------------------------------------------------------------
    /// Sync to disk.
    ///
    /// @return
    ///     An error object that indicates success or the reason for 
    ///     failure.
    //------------------------------------------------------------------
    Error
    Sync ();
    
    //------------------------------------------------------------------
    /// Get the permissions for a this file.
    ///
    /// @return
    ///     Bits logical OR'ed together from the permission bits defined
    ///     in lldb_private::File::Permissions.
    //------------------------------------------------------------------
    uint32_t
    GetPermissions(Error &error) const;
    
    static uint32_t
    GetPermissions(const FileSpec &file_spec, Error &error);

    //------------------------------------------------------------------
    /// Return true if this file is interactive.
    ///
    /// @return
    ///     True if this file is a terminal (tty or pty), false
    ///     otherwise.
    //------------------------------------------------------------------
    bool
    GetIsInteractive ();
    
    //------------------------------------------------------------------
    /// Return true if this file from a real terminal.
    ///
    /// Just knowing a file is a interactive isn't enough, we also need
    /// to know if the terminal has a width and height so we can do
    /// cursor movement and other terminal manipulations by sending
    /// escape sequences.
    ///
    /// @return
    ///     True if this file is a terminal (tty, not a pty) that has
    ///     a non-zero width and height, false otherwise.
    //------------------------------------------------------------------
    bool
    GetIsRealTerminal ();
    
    bool
    GetIsTerminalWithColors ();

    //------------------------------------------------------------------
    /// Output printf formatted output to the stream.
    ///
    /// Print some formatted output to the stream.
    ///
    /// @param[in] format
    ///     A printf style format string.
    ///
    /// @param[in] ...
    ///     Variable arguments that are needed for the printf style
    ///     format string \a format.
    //------------------------------------------------------------------
    size_t
    Printf (const char *format, ...)  __attribute__ ((format (printf, 2, 3)));
    
    size_t
    PrintfVarArg(const char *format, va_list args);

    void
    SetOptions (uint32_t options)
    {
        m_options = options;
    }

protected:
    bool
    DescriptorIsValid () const
    {
        return m_descriptor >= 0;
    }

    bool
    StreamIsValid () const
    {
        return m_stream != kInvalidStream;
    }
    
    void
    CalculateInteractiveAndTerminal ();
    
    //------------------------------------------------------------------
    // Member variables
    //------------------------------------------------------------------
    int m_descriptor;
    FILE *m_stream;
    uint32_t m_options;
    bool m_own_stream;
    bool m_own_descriptor;
    LazyBool m_is_interactive;
    LazyBool m_is_real_terminal;
    LazyBool m_supports_colors;

private:
    DISALLOW_COPY_AND_ASSIGN(File);
};

} // namespace lldb_private

#endif // liblldb_File_h_
