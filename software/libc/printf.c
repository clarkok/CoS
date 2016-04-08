#include <printf.h>

int
sprintf(char *buffer, const char *format, ...)
{
    va_list vl;
    va_start(vl, format);
    return vsprintf(buffer, format, vl);
}

int
vsprintf(char *buffer, const char *format, va_list vlist)
{
    int count = 0;

    while (*format) {
        if (*format == '%') {
            switch (*++format) {
                case 'x':
                    {
                        unsigned int value = va_arg(vlist, unsigned int);
                        static const char ALPHABET[] = "0123456789abcdef";
                        char buf[9];

                        if (!value) {
                            *buffer++ = '0';
                            ++count;
                        }
                        else {
                            for (int i = 0; i < 8; ++i) {
                                buf[i] = ALPHABET[value & 0xF];
                                value >>= 4;
                            }
                            int i = 7;
                            while (buf[i] == '0') --i;
                            while (i >= 0) {
                                *buffer++ = buf[i];
                                --i;
                                ++count;
                            }
                        }
                    }
                    break;
                case 'X':
                    {
                        unsigned int value = va_arg(vlist, unsigned int);
                        static const char ALPHABET[] = "0123456789ABCDEF";
                        char buf[9];

                        if (!value) {
                            *buffer++ = '0';
                            ++count;
                        }
                        else {
                            for (int i = 0; i < 8; ++i) {
                                buf[i] = ALPHABET[value & 0xF];
                                value >>= 4;
                            }
                            int i = 7;
                            while (buf[i] == '0') --i;
                            while (i >= 0) {
                                *buffer++ = buf[i];
                                --i;
                                ++count;
                            }
                        }
                    }
                    break;
                case 'o':
                case 'O':
                    {
                        unsigned int value = va_arg(vlist, unsigned int);
                        static const char ALPHABET[] = "01234567";
                        char buf[12];

                        if (!value) {
                            *buffer++ = '0';
                            ++count;
                        }
                        else {
                            for (int i = 0; i < 12; ++i) {
                                buf[i] = ALPHABET[value & 0xF];
                                value >>= 4;
                            }
                            int i = 11;
                            while (buf[i] == '0') --i;
                            while (i >= 0) {
                                *buffer++ = buf[i];
                                --i;
                                ++count;
                            }
                        }
                    }
                    break;
                case 's':
                    {
                        const char *str = va_arg(vlist, const char *);
                        while (*str) {
                            *buffer++ = *str++;
                            ++count;
                        }
                    }
                    break;
                default:
                    {
                        *buffer++ = *format;
                    }
                    break;
            }
            ++format;
        }
        else {
            *buffer++ = *format++;
            ++count;
        }
    }

    *buffer = '\0';
    return count;
}
