/********************************************************************
* CreatedOn: 2007-10-18   10:13
* FileName: arpcomfunc.h
* CreatedBy: lidengwang <lidengwang@kingsoft.net>
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL:  $
* Purpose:
*********************************************************************/

#pragma once


inline unsigned int build_ip(
    unsigned char a, unsigned char b,
    unsigned char c, unsigned char d)
{
    unsigned int uip = 0;

    uip = d;

    uip = uip << 8;
    uip |= c;

    uip = uip << 8;
    uip |= b;

    uip = uip << 8;
    uip |= a;

    return uip;
}

inline unsigned int build_ip(char * pszip)
{
    int a = 0, b = 0, c = 0, d = 0;
    
    sscanf_s(pszip, " %d . %d . %d . %d ", &a, &b, &c, &d);

    return build_ip(a, b, c, d);
}

inline void build_mac(unsigned char mac[],
    unsigned char a, unsigned char b, unsigned char c, 
    unsigned char d, unsigned char e, unsigned char f)
{
    mac[0] = a;
    mac[1] = b;
    mac[2] = c;
    mac[3] = d;
    mac[4] = e;
    mac[5] = f;
}

inline unsigned char* build_mac(unsigned char mac[], char * pszmac)
{
    int a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
    
    sscanf_s(pszmac, " %x - %x - %x - %x - %x - %x", &a, &b, &c, &d, &e, &f);

    build_mac(mac, a, b, c, d, e, f);

    return mac;
}


//////////////////////////////////////////////////////////////////////////

inline void get_ip(unsigned int uip, 
    unsigned char& a, unsigned char& b, 
    unsigned char& c, unsigned char& d)
{
    a = uip & 0xFF;

    uip = uip >> 8;

    b = uip & 0xFF;

    uip = uip >> 8;

    c = uip & 0xFF;

    uip = uip >> 8;

    d = uip & 0xFF;
}

inline char* get_ip_str(unsigned int uip, char psz_buffer[])
{
    unsigned char a = 0, b = 0, c = 0, d = 0;

    get_ip(uip, a, b, c, d);

    sprintf_s(psz_buffer, 20, "%d.%d.%d.%d", a, b, c, d);

    return psz_buffer;
}

inline char* get_mac_str(unsigned char mac[], char psz_buffer[])
{
    sprintf_s(psz_buffer, 20, "%02x-%02x-%02x-%02x-%02x-%02x", 
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    return psz_buffer;
}

