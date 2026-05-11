#include "http_server.h"
#include "sensor_data.h"
#include "lwip/tcp.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static const char RESP_204[] =
    "HTTP/1.1 204 No Content\r\nConnection: close\r\n\r\n";

/* ------------------------------------------------------------------ */
/* HTML dashboard (served for GET /)                                   */
/* ------------------------------------------------------------------ */

static const char HTML_PAGE[] =
"HTTP/1.1 200 OK\r\n"
"Content-Type: text/html\r\n"
"Connection: close\r\n"
"\r\n"
"<!DOCTYPE html><html lang='en'><head>"
"<meta charset='UTF-8'>"
"<meta name='viewport' content='width=device-width,initial-scale=1'>"
"<title>STM32 Command Center</title>"
"<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>"
"<style>"
"*{box-sizing:border-box;margin:0;padding:0}"
"body{background:#0d1117;color:#e6edf3;font-family:'Segoe UI',sans-serif;min-height:100vh}"
"header{background:#161b22;border-bottom:1px solid #30363d;padding:16px 24px;"
"display:flex;align-items:center;gap:12px}"
"h1{font-size:1.3rem;font-weight:600;color:#58a6ff}"
".dot{width:10px;height:10px;border-radius:50%;background:#3fb950;animation:pulse 1s infinite}"
"@keyframes pulse{0%,100%{opacity:1}50%{opacity:.4}}"
"#ts{margin-left:auto;font-size:.8rem;color:#8b949e}"
"main{padding:24px;max-width:1100px;margin:0 auto}"
".bar{display:flex;gap:8px;margin-bottom:20px;flex-wrap:wrap}"
".tbtn{padding:7px 18px;border-radius:20px;border:1px solid #30363d;"
"background:#21262d;color:#8b949e;cursor:pointer;font-size:.85rem;"
"font-weight:500;transition:all .15s}"
".tbtn.on{border-color:#1f6feb;background:rgba(31,111,235,.13);color:#58a6ff}"
".cards{display:grid;grid-template-columns:repeat(auto-fit,minmax(210px,1fr));"
"gap:16px;margin-bottom:24px}"
".card{background:#161b22;border:1px solid #30363d;border-radius:10px;padding:20px}"
".card.off{display:none}"
".lbl{font-size:.75rem;color:#8b949e;text-transform:uppercase;"
"letter-spacing:.08em;margin-bottom:8px}"
".val{font-size:2rem;font-weight:700}"
".unit{font-size:.9rem;color:#8b949e;margin-left:4px}"
"#c-temp .val{color:#f78166}#c-hum .val{color:#58a6ff}"
"#c-pres .val{color:#3fb950}#c-gas .val{color:#d2a8ff}"
"#csec{background:#161b22;border:1px solid #30363d;border-radius:10px;padding:20px}"
"#csec.off{display:none}"
"#csec h2{font-size:.9rem;color:#8b949e;margin-bottom:16px;"
"text-transform:uppercase;letter-spacing:.08em}"
"canvas{max-height:260px}"
"</style></head><body>"
"<header><div class='dot'></div><h1>STM32 Command Center</h1>"
"<span id='ts'>connecting...</span></header>"
"<main>"
"<div class='bar'>"
"<button class='tbtn' data-s='temp'>&#9679; Temp</button>"
"<button class='tbtn' data-s='hum'>&#9679; Humidity</button>"
"<button class='tbtn' data-s='pres'>&#9679; Pressure</button>"
"<button class='tbtn' data-s='gas'>&#9679; Gas</button>"
"</div>"
"<div class='cards'>"
"<div class='card' id='c-temp'><div class='lbl'>Temperature</div>"
"<div class='val'><span id='vt'>--</span><span class='unit'>&#176;F</span></div></div>"
"<div class='card' id='c-hum'><div class='lbl'>Humidity</div>"
"<div class='val'><span id='vh'>--</span><span class='unit'>%RH</span></div></div>"
"<div class='card' id='c-pres'><div class='lbl'>Pressure</div>"
"<div class='val'><span id='vp'>--</span><span class='unit'>hPa</span></div></div>"
"<div class='card' id='c-gas'><div class='lbl'>Gas Resistance</div>"
"<div class='val'><span id='vg'>--</span><span class='unit'>&#937;</span></div></div>"
"</div>"
"<div id='csec'><h2>Live History</h2><canvas id='ch'></canvas></div>"
"</main>"
"<script>"
"const K=['temp','hum','pres','gas'];"
"const CL={temp:'#f78166',hum:'#58a6ff',pres:'#3fb950',gas:'#d2a8ff'};"
"const UN={temp:'F',hum:'%',pres:'hPa',gas:'ohm'};"
"const LB={temp:'Temp',hum:'Hum',pres:'Pres',gas:'Gas'};"
"const EL={temp:'vt',hum:'vh',pres:'vp',gas:'vg'};"
"const st={};"
"const HL=[],HD={temp:[],hum:[],pres:[],gas:[]};"
"const MAX=60;"
"function load(){K.forEach(k=>{st[k]=localStorage.getItem('s_'+k)!=='0';});}"
"function save(k){localStorage.setItem('s_'+k,st[k]?'1':'0');}"
"function tog(k){st[k]=!st[k];save(k);"
"if(k==='gas')fetch('/set?gas='+(st[k]?'1':'0')).catch(function(){});"
"paint();}"
"function paint(){"
"K.forEach(k=>{"
"const c=document.getElementById('c-'+k);"
"const b=document.querySelector('[data-s='+k+']');"
"if(c)c.classList.toggle('off',!st[k]);"
"if(b)b.classList.toggle('on',st[k]);"
"});"
"document.getElementById('csec').classList.toggle('off',!K.some(k=>st[k]));"
"sync();}"
"const chart=new Chart(document.getElementById('ch'),{"
"type:'line',data:{labels:HL,datasets:[]},"
"options:{animation:false,responsive:true,"
"scales:{x:{ticks:{color:'#8b949e',maxTicksLimit:8},grid:{color:'#21262d'}},"
"y:{ticks:{color:'#8b949e'},grid:{color:'#21262d'}}},"
"plugins:{legend:{labels:{color:'#8b949e',usePointStyle:true}}}}});"
"function sync(){"
"chart.data.datasets=K.filter(k=>st[k]).map(k=>({"
"label:LB[k]+' ('+UN[k]+')',data:HD[k],"
"borderColor:CL[k],backgroundColor:CL[k]+'22',"
"tension:.3,pointRadius:2,fill:k==='temp'}));"
"chart.update('none');}"
"async function poll(){"
"try{"
"const d=await(await fetch('/data')).json();"
"if(d.tf===null)return;"
"const t=new Date().toLocaleTimeString();"
"HL.push(t);"
"HD.temp.push(d.tf);HD.hum.push(d.h);"
"HD.pres.push(d.p);HD.gas.push(d.gv?d.g:null);"
"if(HL.length>MAX){HL.shift();K.forEach(k=>HD[k].shift());}"
"document.getElementById('vt').textContent=d.tf.toFixed(1);"
"document.getElementById('vh').textContent=d.h.toFixed(1);"
"document.getElementById('vp').textContent=d.p.toFixed(1);"
"document.getElementById('vg').textContent=d.gv?Math.round(d.g):'--';"
"document.getElementById('ts').textContent=t;"
"sync();"
"}catch(e){}"
"}"
"document.querySelectorAll('.tbtn').forEach(b=>{"
"b.addEventListener('click',()=>tog(b.getAttribute('data-s')));});"
"load();paint();poll();"
"setInterval(poll,1000);"
"</script></body></html>";

/* ------------------------------------------------------------------ */
/* JSON response builder                                               */
/* ------------------------------------------------------------------ */

static int build_json(char *buf, size_t sz)
{
    /* Fixed-point to avoid float printf — use integer parts */
    int32_t tf_w  = (int32_t)g_sensor.temp_f;
    int32_t tf_f  = (int32_t)((g_sensor.temp_f  - tf_w)  * 10.0f); if (tf_f  < 0) tf_f  = -tf_f;
    int32_t h_w   = (int32_t)g_sensor.humidity;
    int32_t h_f   = (int32_t)((g_sensor.humidity - h_w)  * 10.0f); if (h_f   < 0) h_f   = -h_f;
    int32_t p_w   = (int32_t)g_sensor.pressure_hpa;
    int32_t p_f   = (int32_t)((g_sensor.pressure_hpa - p_w) * 10.0f); if (p_f < 0) p_f = -p_f;
    int32_t g_w   = (int32_t)g_sensor.gas_resistance;

    char body[256];
    int blen;
    if (g_sensor.valid) {
        blen = snprintf(body, sizeof(body),
            "{\"tf\":%ld.%01ld,\"h\":%ld.%01ld,\"p\":%ld.%01ld,\"g\":%ld,\"gv\":%d}",
            (long)tf_w, (long)tf_f,
            (long)h_w,  (long)h_f,
            (long)p_w,  (long)p_f,
            (long)g_w,  (int)g_sensor.gas_valid);
    } else {
        blen = snprintf(body, sizeof(body),
            "{\"tf\":null,\"h\":null,\"p\":null,\"g\":null,\"gv\":0}");
    }

    return snprintf(buf, sz,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        blen, body);
}

/* ------------------------------------------------------------------ */
/* Connection state                                                    */
/* ------------------------------------------------------------------ */

#define MAX_CONNS 3

typedef enum { ST_IDLE, ST_SEND_HTML, ST_SEND_JSON, ST_SEND_204, ST_DONE } ConnState;

typedef struct {
    struct tcp_pcb *pcb;
    ConnState       state;
    const char     *html_ptr;   /* current send position inside HTML_PAGE */
    uint32_t        html_rem;   /* bytes remaining */
    char            json_buf[512];
    uint32_t        json_ptr;
    uint32_t        json_rem;
} Conn;

static Conn s_conns[MAX_CONNS];

static Conn *alloc_conn(struct tcp_pcb *pcb)
{
    for (int i = 0; i < MAX_CONNS; i++) {
        if (s_conns[i].state == ST_IDLE) {
            s_conns[i].pcb   = pcb;
            s_conns[i].state = ST_DONE;   /* caller overwrites */
            return &s_conns[i];
        }
    }
    return NULL;
}

static void free_conn(Conn *c)
{
    c->state = ST_IDLE;
    c->pcb   = NULL;
}

/* ------------------------------------------------------------------ */
/* Chunked send helper                                                 */
/* ------------------------------------------------------------------ */

static void push_data(Conn *c)
{
    struct tcp_pcb *pcb = c->pcb;
    if (!pcb) return;

    const char *ptr;
    uint32_t    rem;

    if (c->state == ST_SEND_HTML) {
        ptr = c->html_ptr;
        rem = c->html_rem;
    } else if (c->state == ST_SEND_JSON) {
        ptr = c->json_buf + c->json_ptr;
        rem = c->json_rem;
    } else {
        return;
    }

    while (rem > 0) {
        uint16_t avail = tcp_sndbuf(pcb);
        if (avail == 0) break;
        uint16_t chunk = (rem < avail) ? (uint16_t)rem : avail;
        if (chunk > 2048) chunk = 2048;

        err_t e = tcp_write(pcb, ptr, chunk, TCP_WRITE_FLAG_COPY);
        if (e != ERR_OK) break;

        ptr += chunk;
        rem -= chunk;
    }
    tcp_output(pcb);

    if (c->state == ST_SEND_HTML) {
        c->html_ptr = ptr;
        c->html_rem = rem;
    } else {
        c->json_ptr = (uint32_t)(ptr - c->json_buf);
        c->json_rem = rem;
    }

    if (rem == 0) {
        tcp_shutdown(pcb, 0, 1);   /* FIN — done sending */
        c->state = ST_DONE;
    }
}

/* ------------------------------------------------------------------ */
/* TCP callbacks                                                       */
/* ------------------------------------------------------------------ */

static err_t sent_cb(void *arg, struct tcp_pcb *pcb, u16_t len)
{
    (void)len;
    Conn *c = (Conn *)arg;
    if (!c) return ERR_OK;
    push_data(c);
    return ERR_OK;
}

static err_t recv_cb(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
    Conn *c = (Conn *)arg;
    if (!c) return ERR_OK;

    if (err != ERR_OK || !p) {
        /* Connection closed by client */
        free_conn(c);
        tcp_close(pcb);
        return ERR_OK;
    }

    tcp_recved(pcb, p->tot_len);

    /* Route: GET / → HTML, GET /data → JSON, GET /set?gas=N → 204 */
    ConnState route = ST_SEND_HTML;
    if (p->len >= 6) {
        char *req = (char *)p->payload;
        if (req[0]=='G' && req[1]=='E' && req[2]=='T' && req[3]==' ' && req[4]=='/') {
            if (req[5]=='d') {
                route = ST_SEND_JSON;
            } else if (req[5]=='s') {
                route = ST_SEND_204;
                /* scan first 64 bytes for gas=N */
                uint16_t scan = p->len < 64 ? p->len : 64;
                for (uint16_t i = 5; i + 4 < scan; i++) {
                    if (req[i]=='g' && req[i+1]=='a' && req[i+2]=='s' && req[i+3]=='=') {
                        Sensor_SetGasEnable(req[i+4] == '1' ? 1 : 0);
                        break;
                    }
                }
            }
        }
    }

    pbuf_free(p);

    if (route == ST_SEND_JSON) {
        int n = build_json(c->json_buf, sizeof(c->json_buf));
        if (n < 0) n = 0;
        c->json_ptr = 0;
        c->json_rem = (uint32_t)n;
        c->state    = ST_SEND_JSON;
    } else if (route == ST_SEND_204) {
        tcp_write(c->pcb, RESP_204, sizeof(RESP_204) - 1, TCP_WRITE_FLAG_COPY);
        tcp_output(c->pcb);
        tcp_shutdown(c->pcb, 0, 1);
        c->state = ST_DONE;
        return ERR_OK;
    } else {
        c->html_ptr = HTML_PAGE;
        c->html_rem = sizeof(HTML_PAGE) - 1;
        c->state    = ST_SEND_HTML;
    }

    push_data(c);
    return ERR_OK;
}

static void err_cb(void *arg, err_t err)
{
    (void)err;
    Conn *c = (Conn *)arg;
    if (c) free_conn(c);
}

static err_t accept_cb(void *arg, struct tcp_pcb *new_pcb, err_t err)
{
    (void)arg;
    if (err != ERR_OK || !new_pcb) return ERR_VAL;

    Conn *c = alloc_conn(new_pcb);
    if (!c) {
        tcp_abort(new_pcb);
        return ERR_ABRT;
    }
    c->state = ST_DONE;   /* waiting for recv */

    tcp_setprio(new_pcb, TCP_PRIO_MIN);
    tcp_arg(new_pcb, c);
    tcp_recv(new_pcb, recv_cb);
    tcp_sent(new_pcb, sent_cb);
    tcp_err(new_pcb, err_cb);
    return ERR_OK;
}

/* ------------------------------------------------------------------ */
/* Public init                                                         */
/* ------------------------------------------------------------------ */

void HTTP_Server_Init(void)
{
    for (int i = 0; i < MAX_CONNS; i++) s_conns[i].state = ST_IDLE;

    struct tcp_pcb *pcb = tcp_new();
    if (!pcb) return;
    tcp_bind(pcb, IP_ADDR_ANY, 80);
    pcb = tcp_listen_with_backlog(pcb, MAX_CONNS);
    if (!pcb) return;
    tcp_accept(pcb, accept_cb);
}
