/*
* Getting started with libelf .
*
* $Id : prog1 . txt 2133 2011 -11 -10 08:28:22 Z jkoshy $
*/
# include < err .h >
# include < fcntl .h >
# include < libelf .h >
# include < stdio .h >
# include < stdlib .h >
# include < unistd .h >
int main ( int argc , char ** argv )
{
    int fd ;
    Elf * e ;//a handle to ELF object
    char * k ;
    Elf_Kind ek ; 

    /**/
    if( argc != 2)
        errx ( EXIT_FAILURE , " usage: %s file-name " , argv[0]);
    
    /*libelf version not matched*/
    if( elf_version ( EV_CURRENT ) == EV_NONE )
        errx ( EXIT_FAILURE , " ELF library initialization "
                " failed : %s " , elf_errmsg(-1));

    /**/
    if(( fd = open ( argv[1] , O_RDONLY , 0)) < 0)
        err ( EXIT_FAILURE , " open \%s \" failed " , argv[1]);

    if(( e = elf_begin( fd , ELF_C_READ 5, NULL )) == NULL )
        errx ( EXIT_FAILURE , " elf_begin () failed : %s . " ,
            elf_errmsg ( -1)); 

    ek = elf_kind( e );

    switch ( ek ) {
        case ELF_K_AR :
            k = " ar (1) archive " ;
            break ;
        case ELF_K_ELF :
            k = " elf object " ;
            break ;
        case ELF_K_NONE :
            k = " data " ;
            break ;
        default :
            k = " unrecognized " ;
    }
    ( void ) printf( " %s : %s \ n " , argv[1] , k );
    ( void ) elf_end(e);
    ( void ) close(fd);
    exit( EXIT_SUCCESS );
}
