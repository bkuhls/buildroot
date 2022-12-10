/*----------------------------------------------------------------------------
 *  netcalc.c - performs various calculations on IP host/network adresses
 *
 *  Copyright (c) 2000-2001 - Frank Meyer <frank@fli4l.de>
 *  Copyright (c) 2001-2016 - fli4l-Team <team@fli4l.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Creation:       07.11.2000  fm
 *  Last Update:    $Id: netcalc.c 44011 2016-01-14 08:34:43Z sklein $
 *----------------------------------------------------------------------------
 */

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define TRUE	1
#define FALSE	0
typedef int BOOL;

#define ERR_UNKNOWN_TYPE          (-1)
#define ERR_INVALID_ADDRESS       (-2)
#define ERR_INVALID_NETMASK       (-3)
#define ERR_INVALID_NETBITMASK    (-4)
#define ERR_CONVERSION_FAILED     (-5)
#define ERR_DNS_CONVERSION_FAILED (-6)
#define ERR_COMBINATION_FAILED    (-7)

/**
 * Represents an address with a netmask.
 */
struct Address {
    /**
     * The address family.
     */
    int type;
    /**
     * The actual address.
     */
    union {
        struct in_addr v4;
        struct in6_addr v6;
    } address;
    /**
     * The netmask bits.
     */
    unsigned netmaskbits;
};

/**
 * Parses an IPv4 address.
 * @param s
 *  The string to parse.
 * @param result
 *  The address where to store the result.
 * @return
 *  TRUE if parsing succeeds, FALSE otherwise.
 */
static BOOL
parseIPv4Address(char const *s, struct Address *result)
{
    result->type = AF_INET;
    result->netmaskbits = 32;
    if (inet_pton(result->type, s, &result->address.v4) == 1) {
        return TRUE;
    }
    else {
        result->type = ERR_INVALID_ADDRESS;
        return FALSE;
    }
}

/**
 * Parses an IPv6 address.
 * @param s
 *  The string to parse.
 * @param result
 *  The address where to store the result.
 * @return
 *  TRUE if parsing succeeds, FALSE otherwise.
 */
static BOOL
parseIPv6Address(char const *s, struct Address *result)
{
    result->type = AF_INET6;
    result->netmaskbits = 128;
    if (inet_pton(result->type, s, &result->address.v6) == 1) {
        return TRUE;
    }
    else {
        result->type = ERR_INVALID_ADDRESS;
        return FALSE;
    }
}

/**
 * Parses an IPv4 or IPv6 address.
 * @param s
 *  The string to parse.
 * @param result
 *  The address where to store the result.
 * @return
 *  TRUE if parsing succeeds, FALSE otherwise.
 */
static BOOL
parseAddressWithoutMask(char *s, struct Address *result)
{
    size_t const len = strlen(s);
    if (strspn(s, "0123456789.") == len) {
        return parseIPv4Address(s, result);
    }
    else if (strspn(s, "0123456789abcdefABCDEF:.") == len) {
        return parseIPv6Address(s, result);
    }
    else {
        result->type = ERR_INVALID_ADDRESS;
        return FALSE;
    }
}

/**
 * Parses an IPv4 or IPv6 address which may contain a CIDR-style netmask.
 * @param s
 *  The string to parse.
 * @param result
 *  The address where to store the result.
 * @return
 *  TRUE if parsing succeeds, FALSE otherwise.
 */
static BOOL
parseAddressWithMask(char *s, struct Address *result)
{
    char *maskbegin = strchr(s, '/');
    if (maskbegin) {
        *maskbegin++ = '\0';
    }

    if (parseAddressWithoutMask(s, result)) {
        unsigned netmaskbits;
        if (maskbegin == NULL) {
            return TRUE;
        }

        if (sscanf(maskbegin, "%u", &netmaskbits) != 1 ||
                netmaskbits > result->netmaskbits) {
            result->type = ERR_INVALID_NETBITMASK;
            return FALSE;
        }
        else {
            result->netmaskbits = netmaskbits;
            return TRUE;
        }
    }
    else {
        return FALSE;
    }
}

/**
 * Returns TRUE if the address is valid.
 * @param addr
 *  The address to check.
 */
static BOOL
isValidAddress(struct Address const *addr)
{
    return addr->type >= 0;
}

/**
 * Builds a CIDR-style netmask from an old-style netmask.
 * @param netmask
 *  The address to transform.
 * @return
 *  TRUE if the address describes a proper CIDR netmask (i.e. where the only
 *  bits set build a block without holes, starting at the MSB; FALSE otherwise.
 */
static BOOL
convertNetmaskToCIDR(struct Address *netmask)
{
    unsigned char const *p;
    int num;
    unsigned char bitmask;
    BOOL exitLoop = FALSE;

    switch (netmask->type) {
    case AF_INET :
        p = (unsigned char const *) &netmask->address.v4.s_addr;
        num = 4;
        break;
    case AF_INET6 :
        p = netmask->address.v6.s6_addr;
        num = 16;
        break;
    default :
        netmask->type = ERR_UNKNOWN_TYPE;
        return FALSE;
    }

    netmask->netmaskbits = 0;
    while (num > 0) {
        for (bitmask = 0x80; bitmask != 0; bitmask >>= 1) {
            if ((*p & bitmask) != 0) {
                ++netmask->netmaskbits;
            }
            else {
                exitLoop = TRUE;
                break;
            }
        }

        if (exitLoop)
            break;

        ++p;
        --num;
    }

    while (num > 0) {
        for (; bitmask != 0; bitmask >>= 1) {
            if ((*p & bitmask) != 0) {
                netmask->type = ERR_INVALID_NETMASK;
                return FALSE;
            }
        }

        ++p;
        --num;
        bitmask = 0x80;
    }

    return TRUE;
}

/**
 * Builds an old-style netmask from a CIDR-style netmask.
 * @param netmask
 *  The address to transform.
 * @return
 *  TRUE if the address describes a proper netmask (i.e. the number of netmask
 *  bits does not exceed the length of the address), FALSE otherwise.
 */
static BOOL
convertCIDRToNetmask(struct Address *netmask)
{
    unsigned char *p;
    int num;
    unsigned char bitmask;
    int bits;

    switch (netmask->type) {
    case AF_INET :
        p = (unsigned char *) &netmask->address.v4.s_addr;
        num = 4;
        break;
    case AF_INET6 :
        p = netmask->address.v6.s6_addr;
        num = 16;
        break;
    default :
        return FALSE;
    }

    memset(p, 0, num);
    bitmask = 0x80;
    bits = (int) netmask->netmaskbits;
    while (bits > 0 && num > 0) {
        *p |= bitmask;
        --bits;
        bitmask >>= 1;
        if (bitmask == 0) {
            bitmask = 0x80;
            ++p;
            --num;
        }
    }

    if (bits > 0) {
        netmask->type = ERR_INVALID_NETBITMASK;
        return FALSE;
    }
    else {
        return TRUE;
    }
}

/**
 * Builds a network address from a CIDR-style address by clearing all bits that
 * do not belong to the net part.
 * @param addr
 *  The address to transform.
 * @return
 *  TRUE if the address is valid, FALSE otherwise.
 */
static BOOL
convertCIDRToNetwork(struct Address *addr)
{
    unsigned char *p;
    int num;
    unsigned char bitmask;
    int bits;

    switch (addr->type) {
    case AF_INET :
        p = (unsigned char *) &addr->address.v4.s_addr;
        num = 4;
        break;
    case AF_INET6 :
        p = addr->address.v6.s6_addr;
        num = 16;
        break;
    default :
        return FALSE;
    }

    p += addr->netmaskbits / 8;
    bits = num * 8 - (int) addr->netmaskbits;
    bitmask = 0x80 >> (addr->netmaskbits % 8);
    while (bits > 0) {
        *p &= ~bitmask;
        --bits;
        bitmask >>= 1;
        if (bitmask == 0) {
            bitmask = 0x80;
            ++p;
        }
    }

    return TRUE;
}

/**
 * Builds a host address from a CIDR-style address by clearing all bits that
 * do not belong to the host part.
 * @param addr
 *  The address to transform.
 * @return
 *  TRUE if the address is valid, FALSE otherwise.
 */
static BOOL
convertCIDRToHost(struct Address *addr)
{
    unsigned char *p;
    unsigned char bitmask;
    int bits;

    switch (addr->type) {
    case AF_INET :
        p = (unsigned char *) &addr->address.v4.s_addr;
        break;
    case AF_INET6 :
        p = addr->address.v6.s6_addr;
        break;
    default :
        return FALSE;
    }

    bits = (int) addr->netmaskbits;
    bitmask = 0x80;
    while (bits > 0) {
        *p &= ~bitmask;
        --bits;
        bitmask >>= 1;
        if (bitmask == 0) {
            bitmask = 0x80;
            ++p;
        }
    }

    return TRUE;
}

/**
 * Builds an IPv4 broadcast address from a CIDR-style address by setting all
 * bits that belong to the host part.
 * @param addr
 *  The address to transform.
 * @return
 *  TRUE if the address is valid, FALSE otherwise.
 */
static BOOL
convertCIDRToBroadcast(struct Address *addr)
{
    unsigned char *p;
    int num;
    unsigned char bitmask;
    int bits;

    switch (addr->type) {
    case AF_INET :
        p = (unsigned char *) &addr->address.v4.s_addr;
        num = 4;
        break;
    case AF_INET6 :
        p = addr->address.v6.s6_addr;
        num = 16;
        break;
    default :
        return FALSE;
    }

    p += addr->netmaskbits / 8;
    bits = num * 8 - (int) addr->netmaskbits;
    bitmask = 0x80 >> (addr->netmaskbits % 8);
    while (bits > 0) {
        *p |= bitmask;
        --bits;
        bitmask >>= 1;
        if (bitmask == 0) {
            bitmask = 0x80;
            ++p;
        }
    }

    return TRUE;
}

/**
 * Converts a number to a string.
 * @param value
 *  The value to convert.
 * @param maxDigits
 *  The maximum number of digits that can occur.
 * @param format
 *  The format to use (e.g. "%u" or "%x").
 * @return
 *  A dynamically allocated buffer with the string or NULL if an error occurred.
 */
static char *
convertNumberToString(unsigned value, size_t maxDigits, char const *format)
{
    size_t const bufLen = maxDigits + 1;
    char *const buf = (char *) malloc(bufLen);
    if (snprintf(buf, bufLen, format, value) > 0) {
        return buf;
    } else {
        free(buf);
        return NULL;
    }
}

/**
 * Prepends a string in front of another one.
 * @param buf
 *  The original string in a dynamically allocated buffer. May be NULL.
 * @param prefix
 *  The string to be prepended.
 * @result
 *  <prefix>+<buf>, where "+" is the string concatenation.
 */
static char *
prependString(char *buf, char const *prefix)
{
    size_t const bufLen = buf ? strlen(buf) : 0;
    size_t const prefixLen = strlen(prefix);
    buf = realloc(buf, bufLen + prefixLen + 1);
    memmove(buf + prefixLen, buf, bufLen);
    strncpy(buf, prefix, prefixLen);
    buf[bufLen + prefixLen] = '\0';
    return buf;
}

/**
 * Builds a reverse DNS name for an IPv4 or IPv6 address.
 * @param addr
 *  The address to transform.
 */
static void
convertCIDRToDNSRev(struct Address *addr)
{
    unsigned char const *p;
    int bitsPerEntry; /* must be divisor of 8 == sizeof(char)! */
    size_t maxChars;  /* maximum characters in resulting name entry plus one
                         for the "." */
    char const *format;   /* printf format string, including the "." */
    char const *suffix;   /* domain to be appended to the resulting name */
    unsigned initialMask; /* initial mask for the leftmost part of a byte */
    unsigned mask;        /* current mask for the current part of a byte */
    unsigned initialShiftCount;  /* initial shift count for leftmost part */
    unsigned shiftCount;  /* current shift count for current part */
    unsigned bits = addr->netmaskbits;  /* bits to process */
    char *result = NULL;  /* resulting string, dynamically allocated */

    switch (addr->type) {
    case AF_INET :
        p = (unsigned char const *) &addr->address.v4.s_addr;
        bitsPerEntry = 8; /* each DNS name component describes a byte */
        maxChars = 3 + 1; /* max. three digits per byte + "." */
        format = "%u.";   /* print decimal number */
        suffix = "in-addr.arpa";
        break;
    case AF_INET6 :
        p = addr->address.v6.s6_addr;
        bitsPerEntry = 4; /* each DNS name component describes a nibble */
        maxChars = 1 + 1; /* max. one (hexadecimal) digit per byte + "." */
        format = "%x.";   /* print hexadecimal number */
        suffix = "ip6.arpa";
        break;
    }

    initialMask = (unsigned char) ~(((unsigned char) ~0u) >> bitsPerEntry);
    initialShiftCount = ((8 / bitsPerEntry) - 1) * bitsPerEntry;

    mask = initialMask;
    shiftCount = initialShiftCount;
    while (bits >= bitsPerEntry) {
        unsigned value;
        char *str;

        /* mask and shift */
        value = (*p & mask) >> shiftCount;
        str = convertNumberToString(value, maxChars, format);
        if (str) {
            result = prependString(result, str);
            free(str);
        }

        /* adjust mask */
        mask >>= bitsPerEntry;
        bits -= bitsPerEntry;

        if (mask == 0) {
            /* reset mask and shift count to initial values */
            mask = initialMask;
            shiftCount = initialShiftCount;
            /* advance to next byte */
            ++p;
        } else {
            /* process next part */
            shiftCount -= bitsPerEntry;
        }
    }

    if (bits > 0) {
        unsigned startValue;
        unsigned value;
        char *str;

        /* compute mask for remaining bits */
        unsigned bit = 0x1u << shiftCount;
        while (bits++ < bitsPerEntry) {
            mask &= ~bit;
            bit <<= 1;
        }

        /* start value for iteration */
        startValue = (*p & mask) >> shiftCount;
        mask >>= shiftCount;
        value = startValue;
        /* loop ends if incrementing value leaves range covered by mask */
        while ((value & mask) == startValue) {
            char *str = convertNumberToString(value, maxChars, format);
            if (str) {
                printf("%s%s%s\n", str, result, suffix);
                free(str);
            }

            ++value;
        }
    } else if (result) {
        printf("%s%s\n", result, suffix);
    }

    free(result);
}

/**
 * Combines a network and a host address by OR-ing all bits.
 * @param addr
 *  The address to transform.
 * @return
 *  TRUE if the net and host addresses are valid and do not overlap, FALS
 *  otherwise.
 */
static BOOL
combineAddresses(struct Address *network, struct Address const *host)
{
    unsigned char *p1;
    unsigned char const *p2;
    unsigned char const *pnm;
    int num;
    struct Address netmask = *network;

    switch (network->type) {
    case AF_INET :
        p1 = (unsigned char *) &network->address.v4.s_addr;
        p2 = (unsigned char const *) &host->address.v4.s_addr;
        pnm = (unsigned char const *) &netmask.address.v4.s_addr;
        num = 4;
        break;
    case AF_INET6 :
        p1 = network->address.v6.s6_addr;
        p2 = host->address.v6.s6_addr;
        pnm = netmask.address.v6.s6_addr;
        num = 16;
        break;
    default :
        return FALSE;
    }

    if (!convertCIDRToNetmask(&netmask)) {
        return FALSE;
    }

    while (num-- > 0) {
        if ((*pnm & *p2) != 0) {
            network->type = ERR_COMBINATION_FAILED;
            return FALSE;
        }
        *p1 = (*p1 & *pnm) | *p2;
        ++p1;
        ++p2;
        ++pnm;
    }

    return TRUE;
}

/**
 * Transforms an address into a canonical string representation.
 * @param addr
 *  The address to transform.
 * @return
 *  Points to the resulting address string or NULL if an error occurs (e.g. if
 *  the address passed is invalid or if memory cannot be allocated).
 */
static char *
addressToString(struct Address const *addr)
{
    char *buf;
    size_t bufsize;
    void const *src;

    switch (addr->type) {
    case AF_INET :
        bufsize = 4 * 4;
        src = &addr->address.v4;
        break;
    case AF_INET6 :
        bufsize = 8 * 5;
        src = &addr->address.v6;
        break;
    default :
        return NULL;
    }

    buf = (char *) malloc(bufsize);
    if (!buf) {
        return NULL;
    }
    if (!inet_ntop(addr->type, src, buf, bufsize)) {
        free(buf);
        buf = NULL;
    }
    return buf;
}

/**
 * Presents the result to the caller of this program and print an appropriate
 * error message if necessary.
 * @param result
 *  The result to present.
 * @param err
 *  If the result is invalid, this is the argument that caused the failure.
 * @param includeNetmask
 *  If TRUE, the netmask is included in the output, otherwise it is left out.
 * @return
 *  The exit code of the application.
 */
static int
makeResult(struct Address const *result, char const *err, BOOL includeNetmask)
{
    if (result->type < 0) {
        switch (result->type) {
        case ERR_INVALID_ADDRESS :
            fprintf(stderr, "invalid address '%s'\n", err);
            break;
        case ERR_INVALID_NETBITMASK :
            /* ugly hack in order to access the netbitmask */
            fprintf(stderr, "invalid CIDR netmask '%s'\n", err + strlen(err) + 1);
            break;
        case ERR_INVALID_NETMASK :
            fprintf(stderr, "invalid netmask '%s'\n", err);
            break;
        case ERR_COMBINATION_FAILED :
            fprintf(stderr, "address combination failed due to overlapping\n");
            break;
        default :
            fprintf(stderr, "unknown error %d in '%s'\n", result->type, err);
            break;
        }
        return -result->type + 1;
    }
    else {
        char *str = addressToString(result);
        if (str) {
            if (includeNetmask) {
                printf("%s/%u\n", str, result->netmaskbits);
            }
            else {
                printf("%s\n", str);
            }
            free(str);
            return 0;
        }
        else {
            fprintf(stderr, "address conversion failed for '%s'\n", err);
            return -ERR_CONVERSION_FAILED + 1;
        }
    }
}

/**
 * Prints usage information.
 * @param pgm_name
 *  The program name.
 * @return
 *  The exit code.
 */
static int
usage(char const *pgm_name)
{
    fprintf (stderr, "usage:\n");
    putc ('\n', stderr);
    fprintf (stderr, "  canonicalize address:\n");
    fprintf (stderr, "    %s canonicalize IPADDR\n", pgm_name);
    putc ('\n', stderr);
    fprintf (stderr, "  check address validity:\n");
    fprintf (stderr, "    %s isip IPADDR\n", pgm_name);
    fprintf (stderr, "    %s isipv4 IPV4ADDR\n", pgm_name);
    fprintf (stderr, "    %s isipv6 IPV6ADDR\n", pgm_name);
    putc ('\n', stderr);
    fprintf (stderr, "  print broadcast address:\n");
    fprintf (stderr, "    %s broadcast IPADDR NETMASK\n", pgm_name);
    fprintf (stderr, "    %s broadcast IPADDR/NETMASKBITS\n", pgm_name);
    putc ('\n', stderr);
    fprintf (stderr, "  print netmask:\n");
    fprintf (stderr, "    %s netmask IPADDR/NETMASKBITS\n", pgm_name);
    putc ('\n', stderr);
    fprintf (stderr, "  print netmaskbits:\n");
    fprintf (stderr, "    %s netmaskbits NETMASK\n", pgm_name);
    fprintf (stderr, "    %s netmaskbits IPADDR/NETMASKBITS\n", pgm_name);
    putc ('\n', stderr);
    fprintf (stderr, "  print network part of address:\n");
    fprintf (stderr, "    %s network IPADDR NETMASK\n", pgm_name);
    fprintf (stderr, "    %s network IPADDR/NETMASKBITS\n", pgm_name);
    putc ('\n', stderr);
    fprintf (stderr, "  print host part of address:\n");
    fprintf (stderr, "    %s host IPADDR NETMASK\n", pgm_name);
    fprintf (stderr, "    %s host IPADDR/NETMASKBITS\n", pgm_name);
    putc ('\n', stderr);
    fprintf (stderr, "  combine network and host parts:\n");
    fprintf (stderr, "    %s combine IPADDR NETMASK IPADDR2\n", pgm_name);
    fprintf (stderr, "    %s combine IPADDR/NETMASKBITS IPADDR2\n", pgm_name);
    putc ('\n', stderr);
    fprintf (stderr, "  compute reverse DNS names:\n");
    fprintf (stderr, "    %s dnsrev IPADDR NETMASK\n", pgm_name);
    fprintf (stderr, "    %s dnsrev IPADDR/NETMASKBITS\n", pgm_name);
    putc ('\n', stderr);

    return 1;
}

/**
 * Main entry point.
 * @param argc
 *  The number of arguments.
 * @param argv
 *  The array containing all arguments.
 */
int
main(int argc, char *argv[])
{
    struct Address addr;
    char const *err = NULL;
    int minArg;

    if (argc < 3 || argc > 5) {
        return usage(argv[0]);
    }

    if (strcmp(argv[1], "isip") == 0) {
        if (argc == 3) {
            parseAddressWithoutMask(argv[2], &addr);
            return isValidAddress(&addr) ? 0 : 1;
        }
        else {
            return usage(argv[0]);
        }
    }

    if (strcmp(argv[1], "isipv4") == 0) {
        if (argc == 3) {
            parseAddressWithoutMask(argv[2], &addr);
            return addr.type == AF_INET ? 0 : 1;
        }
        else {
            return usage(argv[0]);
        }
    }

    if (strcmp(argv[1], "isipv6") == 0) {
        if (argc == 3) {
            parseAddressWithoutMask(argv[2], &addr);
            return addr.type == AF_INET6 ? 0 : 1;
        }
        else {
            return usage(argv[0]);
        }
    }

    if (strcmp(argv[1], "netmask") == 0) {
        if (argc == 3) {
            if (parseAddressWithMask(argv[2], &addr)) {
                convertCIDRToNetmask(&addr);
            }
            return makeResult(&addr, argv[2], FALSE);
        }
        else {
            return usage(argv[0]);
        }
    }

    if (strcmp(argv[1], "netmaskbits") == 0) {
        if (argc == 3) {
            if (parseAddressWithoutMask(argv[2], &addr)) {
                convertNetmaskToCIDR(&addr);
            }
            else {
                parseAddressWithMask(argv[2], &addr);
            }
            if (isValidAddress(&addr)) {
                printf("%u\n", addr.netmaskbits);
                return 0;
            }
            else {
                return makeResult(&addr, argv[2], FALSE);
            }
        }
        else {
            return usage(argv[0]);
        }
    }

    if (strcmp(argv[1], "combine") == 0) {
        minArg = 4;
    }
    else {
        minArg = 3;
    }

    if (argc < minArg || argc > minArg + 1) {
        return usage(argv[0]);
    }
    else if (argc == minArg) {
        parseAddressWithMask(argv[2], &addr);
        err = argv[2];
    }
    else {
        if (parseAddressWithoutMask(argv[2], &addr)) {
            struct Address mask;
            if (parseAddressWithoutMask(argv[3], &mask)
                    && convertNetmaskToCIDR(&mask)) {
                if (addr.type == mask.type) {
                    addr.netmaskbits = mask.netmaskbits;
                }
                else {
                    fprintf(stderr, "address and netmask must belong to the same address family\n");
                    return -ERR_INVALID_NETMASK + 1;
                }
            }
            else {
                err = argv[3];
                addr.type = mask.type == ERR_INVALID_ADDRESS
                    ? ERR_INVALID_NETMASK : mask.type;
            }
        }
        else if (parseAddressWithMask(argv[2], &addr)) {
            return usage(argv[0]);
        }
        else {
            err = argv[2];
        }
    }

    if (strcmp(argv[1], "dnsrev") == 0) {
        if (isValidAddress(&addr)) {
            convertCIDRToDNSRev(&addr);
            return 0;
        }
        return makeResult(&addr, err, FALSE);
    }

    if (strcmp(argv[1], "canonicalize") == 0) {
        return makeResult(&addr, err, TRUE);
    }

    if (strcmp(argv[1], "network") == 0) {
        if (isValidAddress(&addr)) {
            convertCIDRToNetwork(&addr);
            err = argv[2];
        }
        return makeResult(&addr, err, FALSE);
    }

    if (strcmp(argv[1], "host") == 0) {
        if (isValidAddress(&addr)) {
            convertCIDRToHost(&addr);
            err = argv[2];
        }
        return makeResult(&addr, err, FALSE);
    }

    if (strcmp(argv[1], "broadcast") == 0) {
        if (addr.type == AF_INET6) {
            printf("ff02::1\n");
            return 0;
        }
        else {
            if (isValidAddress(&addr)) {
                convertCIDRToBroadcast(&addr);
                err = argv[2];
            }
            return makeResult(&addr, err, FALSE);
        }
    }

    if (strcmp(argv[1], "combine") == 0) {
        struct Address addr2;
        if (isValidAddress(&addr)) {
            if (parseAddressWithoutMask(argv[argc - 1], &addr2)) {
                if (addr.type == addr2.type) {
                    combineAddresses(&addr, &addr2);
                    return makeResult(&addr, argv[2], TRUE);
                }
                else {
                    fprintf(stderr, "addresses to be combined must belong to the same address family\n");
                    return -ERR_INVALID_ADDRESS + 1;
                }
            }
            else {
                return makeResult(&addr2, argv[argc - 1], FALSE);
            }
        }
        else {
            return makeResult(&addr, err, FALSE);
        }
    }

    return usage(argv[0]);
}
