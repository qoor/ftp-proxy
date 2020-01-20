#ifndef __CLIENT_H__
#define __CLIENT_H__

#define BIND_PORT 7777 /* 클라이언트가 로드 밸런스에게 연결할 기본 포트*/
#define MAX_EVENTS 254 /* 모니터링 할 파일 디스크립터의 수 지정 */

struct ip_hdr
{
#if __BYTE_ORDER__ == __LITTLE_ENDIAN
    uint8_t ip_hdr_len:4; /* IP HEADER LENGTH */
    uint8_t ip_version:4; /* IP VERSION */
#else
    uint8_t ip_version:4;
    uint8_t ip_hdr_len:4;
#endif
    uint8_t ip_tos; /* TOS FIELD */
    uint16_t ip_len; /* PAYLOAD FIELD = HEADER + SDU */

    uint16_t ip_id; /* IDENTIFICATION FIELD */
    uint16_t ip_off; /* FLAG(DF, MF) + FRAGMENT OFFSET FIELD */

    uint8_t ip_ttl; /* TIME TO LIVE */
    uint8_t ip_proto; /* UPPER LAYER PROTOCOL */
    uint16_t ip_check; /* IP CHECKSUM */

    uint32_t ip_src; /* SOURCE ADDRESS */
    uint16_t ip_dst; /* DESTINATION ADDRESS */

};

void error_proc(const char* str);
void create_epoll(void);
void create_socket(void);
void listen_epoll(void);
#endif