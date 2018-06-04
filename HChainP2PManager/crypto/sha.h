#ifndef _SHA_PRIVATE__H
#define _SHA_PRIVATE__H


#ifndef USE_MODIFIED_MACROS
#define SHA_Ch(x,y,z)        (((x) & (y)) ^ ((~(x)) & (z)))
#define SHA_Maj(x,y,z)       (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

#else 

#define SHA_Ch(x, y, z)      (((x) & ((y) ^ (z))) ^ (z))
#define SHA_Maj(x, y, z)     (((x) & ((y) | (z))) | ((y) & (z)))
#endif 

#define SHA_Parity(x, y, z)  ((x) ^ (y) ^ (z))
#if WORD <= 32
#define USE_32BIT_ONLY
#endif

#endif 
#ifndef _SHA_H_
#define _SHA_H_

#include <stdint.h>


#ifndef _SHA_enum_
#define _SHA_enum_

enum {
    shaSuccess = 0,
    shaNull,            
    shaInputTooLong,    
    shaStateError,      
    shaBadParam        
};
#endif 


enum {
    SHA1_Message_Block_Size = 64, SHA224_Message_Block_Size = 64,
    SHA256_Message_Block_Size = 64, SHA384_Message_Block_Size = 128,
    SHA512_Message_Block_Size = 128,
    USHA_Max_Message_Block_Size = SHA512_Message_Block_Size,

    SHA1HashSize = 20, SHA224HashSize = 28, SHA256HashSize = 32,
    SHA384HashSize = 48, SHA512HashSize = 64,
    USHAMaxHashSize = SHA512HashSize,

    SHA1HashSizeBits = 160, SHA224HashSizeBits = 224,
    SHA256HashSizeBits = 256, SHA384HashSizeBits = 384,
    SHA512HashSizeBits = 512, USHAMaxHashSizeBits = SHA512HashSizeBits
};


typedef enum SHAversion {
    SHA1, SHA224, SHA256, SHA384, SHA512
} SHAversion;


typedef struct SHA1Context {
    uint32_t Intermediate_Hash[SHA1HashSize/4]; 

    uint32_t Length_Low;                
    uint32_t Length_High;               

    int_least16_t Message_Block_Index;  
                                        
    uint8_t Message_Block[SHA1_Message_Block_Size];

    int Computed;                       
    int Corrupted;                      
} SHA1Context;


typedef struct SHA256Context {
    uint32_t Intermediate_Hash[SHA256HashSize/4]; 

    uint32_t Length_Low;                
    uint32_t Length_High;               

    int_least16_t Message_Block_Index;  
                                        
    uint8_t Message_Block[SHA256_Message_Block_Size];

    int Computed;                      
    int Corrupted;                      
} SHA256Context;

typedef struct SHA512Context {
#ifdef USE_32BIT_ONLY
    uint32_t Intermediate_Hash[SHA512HashSize/4]; 
    uint32_t Length[4];                 
#else 
    uint64_t Intermediate_Hash[SHA512HashSize/8]; 
    uint64_t Length_Low, Length_High;   
#endif 
    int_least16_t Message_Block_Index;  
                                        
    uint8_t Message_Block[SHA512_Message_Block_Size];

    int Computed;                       
    int Corrupted;                      
} SHA512Context;


typedef struct SHA256Context SHA224Context;


typedef struct SHA512Context SHA384Context;


typedef struct USHAContext {
    int whichSha;               
    union {
      SHA1Context sha1Context;
      SHA224Context sha224Context; SHA256Context sha256Context;
      SHA384Context sha384Context; SHA512Context sha512Context;
    } ctx;
} USHAContext;


typedef struct HMACContext {
    int whichSha;               
    int hashSize;               
    int blockSize;              
    USHAContext shaContext;    
    unsigned char k_opad[USHA_Max_Message_Block_Size];
                        
} HMACContext;


extern int SHA1Reset(SHA1Context *);
extern int SHA1Input(SHA1Context *, const uint8_t *bytes,
                     unsigned int bytecount);
extern int SHA1FinalBits(SHA1Context *, const uint8_t bits,
                         unsigned int bitcount);
extern int SHA1Result(SHA1Context *,
                      uint8_t Message_Digest[SHA1HashSize]);

extern int SHA224Reset(SHA224Context *);
extern int SHA224Input(SHA224Context *, const uint8_t *bytes,
                       unsigned int bytecount);
extern int SHA224FinalBits(SHA224Context *, const uint8_t bits,
                           unsigned int bitcount);
extern int SHA224Result(SHA224Context *,
                        uint8_t Message_Digest[SHA224HashSize]);

extern int SHA256Reset(SHA256Context *);
extern int SHA256Input(SHA256Context *, const uint8_t *bytes,
                       unsigned int bytecount);
extern int SHA256FinalBits(SHA256Context *, const uint8_t bits,
                           unsigned int bitcount);
extern int SHA256Result(SHA256Context *,
                        uint8_t Message_Digest[SHA256HashSize]);

extern int SHA384Reset(SHA384Context *);
extern int SHA384Input(SHA384Context *, const uint8_t *bytes,
                       unsigned int bytecount);
extern int SHA384FinalBits(SHA384Context *, const uint8_t bits,
                           unsigned int bitcount);
extern int SHA384Result(SHA384Context *,
                        uint8_t Message_Digest[SHA384HashSize]);

extern int SHA512Reset(SHA512Context *);
extern int SHA512Input(SHA512Context *, const uint8_t *bytes,
                       unsigned int bytecount);
extern int SHA512FinalBits(SHA512Context *, const uint8_t bits,
                           unsigned int bitcount);
extern int SHA512Result(SHA512Context *,
                        uint8_t Message_Digest[SHA512HashSize]);

#endif 
