#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "crazy_snprintf.h"

/**
    @fn crazy_snprintf_impl
    @brief Printf diferenciado

    @param buf Ponteiro para buffer onde a mensagem formatada ser� armazenada
    @param max Tamanho dispon�vel no buffer
    @param ... Formato e argumentos

    @note N�o use essa fun��o diretamente, prefica a macro CRAZY_SNPRINTF()

    O formato e argumentos pode ser:
        <str>                   String pura, sem argumentos
        <fmt>,<val>             String de formato com argumento �nico
        <fmt>,<val>,<val>...    String de formato com multiplos argumentos

    Exemplo:

        len = CRAZY_SNPRINTF(buf, BUFMAX, "testando");
        buf[len] = 0;
        // sa�da ser� "testando"

        len = CRAZY_SNPRINTF(buf, BUFMAX, "teste %d", 1);
        buf[len] = 0;
        // sa�da ser� "teste 1"

        len = CRAZY_SNPRINTF(buf, BUFMAX, "abcd%c %s", 'e', "2");
        buf[len] = 0;
        // sa�da ser� "abcde 2"

        len = CRAZY_SNPRINTF(buf, BUFMAX, "fghij %d", 3, "/%d", 4);
        buf[len] = 0;
        // sa�da ser� "fghij 3/4"

        len = CRAZY_SNPRINTF(buf, BUFMAX, "doido %d.%d", 5, 6, " l� ele");
        buf[len] = 0;
        // sa�da ser� "doido 5.6 l� ele"

    Essa rotina permite que a implementa��o das mensagens formatadas seja mais
    leg�vel (principalmente quando h� uma grande quantidade de par�metros).
    Por exemplo, o c�digo

        len = snprintf(buf, BUFMAX,
            "ip:%d.%d.%d.%d v%d.%d %s %08x",
            ip[0], ip[1], ip[2], ip[3],
            version, release,
            sql_connected() ? "conectado" : "n�o conectado",
            registration_code()
        );

    poder� ser alterado para

        len = CRAZY_SNPRINTF(buf, BUFMAX,
            "ip:%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3],
            "v%d.%d", version, release,
            "%s", sql_connected() ? "conectado" : "n�o conectado",
            "%08x", registration_code()
        );
 */
uint32_t crazy_snprintf_impl( char *buf, uint32_t max, ... ) {
    uint32_t len = 0;
    va_list ap;
    char *fmt = NULL;
    char *value = NULL;

    buf[0] = 0;
    va_start( ap, max );

    while ( ( fmt = va_arg( ap, char * ) ) != NULL ) {
        // Conta na string de formato se h� outros formatos. � aqui que a coisa
        // doida acontece.
        // Geralmente teriamos os par�metros em pares, sendo <fmt>,<val>.
        // Mas, como gostamos de complicar, podemos ter os par�metros com mais
        // de um formato fazendo <fmt>,<val0>,<valn>.. e em seguida novos <fmt>.
        // Para conseguir consumir esses formatos precisamos contar a quantidade
        // de caracteres de formato '%' v�lidos.
        int nargs = 0;
        char *s = fmt;
        for ( ; s[nargs]; (void)*s++ ) {
            if ( s[nargs] == '%' ) {
                if ( s[nargs + 1] != '%' ) {
                    nargs++;
                }
                else {
                    // Aqui poss�velmente � um "%%" utilizado para conseguir
                    // adicionar o caratere '%'
                    (void)*s++;
                }
            }
        }

        // Processa o formato e argumentos e interrompe o processo se ocorrer
        // algum erro. Se o vsnprintf retornou erro, ent�o n�o temos como
        // garantir que os pr�ximos argumentos ser�o processados corretamente.
        int ret = vsnprintf( &buf[len], max - len, fmt, ap );
        if ( ret < 0 ) {
            break;
        }
        len += ret;
        buf[len] = 0;

        // Descarta os argumentos j� utilizados por vsnprintf.
        // Isso � necess�rio pois vsnprintf utiliza os argumentos da va_list mas
        // n�o os consome.
        for ( int i = 0; i < nargs; i++ )
            value = va_arg( ap, void * );
    }

    va_end( ap );

    return len;
}
