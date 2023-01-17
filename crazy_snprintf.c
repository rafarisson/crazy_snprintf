#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "crazy_snprintf.h"

/**
    @fn crazy_snprintf_impl
    @brief Printf diferenciado

    @param buf Ponteiro para buffer onde a mensagem formatada será armazenada
    @param max Tamanho disponível no buffer
    @param ... Formato e argumentos

    @note Não use essa função diretamente, prefica a macro CRAZY_SNPRINTF()

    O formato e argumentos pode ser:
        <str>                   String pura, sem argumentos
        <fmt>,<val>             String de formato com argumento único
        <fmt>,<val>,<val>...    String de formato com multiplos argumentos

    Exemplo:

        len = CRAZY_SNPRINTF(buf, BUFMAX, "testando");
        buf[len] = 0;
        // saída será "testando"

        len = CRAZY_SNPRINTF(buf, BUFMAX, "teste %d", 1);
        buf[len] = 0;
        // saída será "teste 1"

        len = CRAZY_SNPRINTF(buf, BUFMAX, "abcd%c %s", 'e', "2");
        buf[len] = 0;
        // saída será "abcde 2"

        len = CRAZY_SNPRINTF(buf, BUFMAX, "fghij %d", 3, "/%d", 4);
        buf[len] = 0;
        // saída será "fghij 3/4"

        len = CRAZY_SNPRINTF(buf, BUFMAX, "doido %d.%d", 5, 6, " lá ele");
        buf[len] = 0;
        // saída será "doido 5.6 lá ele"

    Essa rotina permite que a implementação das mensagens formatadas seja mais
    legível (principalmente quando há uma grande quantidade de parâmetros).
    Por exemplo, o código

        len = snprintf(buf, BUFMAX,
            "ip:%d.%d.%d.%d v%d.%d %s %08x",
            ip[0], ip[1], ip[2], ip[3],
            version, release,
            sql_connected() ? "conectado" : "não conectado",
            registration_code()
        );

    poderá ser alterado para

        len = CRAZY_SNPRINTF(buf, BUFMAX,
            "ip:%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3],
            "v%d.%d", version, release,
            "%s", sql_connected() ? "conectado" : "não conectado",
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
        // Conta na string de formato se há outros formatos. É aqui que a coisa
        // doida acontece.
        // Geralmente teriamos os parâmetros em pares, sendo <fmt>,<val>.
        // Mas, como gostamos de complicar, podemos ter os parâmetros com mais
        // de um formato fazendo <fmt>,<val0>,<valn>.. e em seguida novos <fmt>.
        // Para conseguir consumir esses formatos precisamos contar a quantidade
        // de caracteres de formato '%' válidos.
        int nargs = 0;
        char *s = fmt;
        for ( ; s[nargs]; (void)*s++ ) {
            if ( s[nargs] == '%' ) {
                if ( s[nargs + 1] != '%' ) {
                    nargs++;
                }
                else {
                    // Aqui possívelmente é um "%%" utilizado para conseguir
                    // adicionar o caratere '%'
                    (void)*s++;
                }
            }
        }

        // Processa o formato e argumentos e interrompe o processo se ocorrer
        // algum erro. Se o vsnprintf retornou erro, então não temos como
        // garantir que os próximos argumentos serão processados corretamente.
        int ret = vsnprintf( &buf[len], max - len, fmt, ap );
        if ( ret < 0 ) {
            break;
        }
        len += ret;
        buf[len] = 0;

        // Descarta os argumentos já utilizados por vsnprintf.
        // Isso é necessário pois vsnprintf utiliza os argumentos da va_list mas
        // não os consome.
        for ( int i = 0; i < nargs; i++ )
            value = va_arg( ap, void * );
    }

    va_end( ap );

    return len;
}
