#include <sys/types.h>
extern "C" {
#include <keyutils.h>
}
#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "key_manager.h"

int cctv_request_key(const char *desc, unsigned char *key, int *len)
{
    constexpr unsigned int KEY_SIZE = 256;
    char auth_key_payload[KEY_SIZE] = { 0, };
    int akp_size = 0;
    long auth_key = 0;
    long ret = -1; 

    /*
     * Workaround on keyctl()
     */
    ret = keyctl_link(KEY_SPEC_USER_KEYRING, KEY_SPEC_SESSION_KEYRING);
    if (ret != 0) {
        perror("keyctl_link");
        return -1;
    } else {
        printf("link user to session keyring\n");
    }   

    auth_key = request_key("user", desc, NULL, KEY_SPEC_SESSION_KEYRING);
    if (auth_key == ENOKEY)
        return -1; 
    //printf("Auth key ID:          %lu\n", (long) auth_key);

    akp_size = keyctl(KEYCTL_READ, auth_key,
             auth_key_payload, sizeof(auth_key_payload));
    if (akp_size == -1) {
        printf("KEYCTL_READ failed: %s\n", strerror(errno));
        return -1; 
    }   
    *len = akp_size;

    if (akp_size >= KEY_SIZE)
        return -1; 
    memcpy(key, auth_key_payload, akp_size);
    auth_key_payload[akp_size] = '\0';
    
    return 0;
}