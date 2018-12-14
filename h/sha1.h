#ifndef LWIP_INCLUDED_POLARSSL_SHA1_H
#define LWIP_INCLUDED_POLARSSL_SHA1_H

/**
 * \brief          SHA-1 context structure
 */
typedef struct
{
    unsigned long total[2];     /*!< number of bytes processed  */
    unsigned long state[5];     /*!< intermediate digest state  */
    unsigned char buffer[64];   /*!< data block being processed */
}
sha1_context;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          SHA-1 context setup
 *
 * \param ctx      context to be initialized
 */
void sha1_starts( sha1_context *ctx );

/**
 * \brief          SHA-1 process buffer
 *
 * \param ctx      SHA-1 context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void sha1_update( sha1_context *ctx, const unsigned char *input, int ilen );

/**
 * \brief          SHA-1 final digest
 *
 * \param ctx      SHA-1 context
 * \param output   SHA-1 checksum result
 */
void sha1_finish( sha1_context *ctx, unsigned char output[20] );

/**
 * \brief          Output = SHA-1( input buffer )
 *
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   SHA-1 checksum result
 */
void sha1( unsigned char *input, int ilen, unsigned char output[20] );

#ifdef __cplusplus
}
#endif

#endif /* LWIP_INCLUDED_POLARSSL_SHA1_H */
