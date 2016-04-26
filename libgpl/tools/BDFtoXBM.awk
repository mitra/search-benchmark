/FONT/ {
    if ($1 == "FONT") {
        printf ("	/* %s */\n", $0) ;
    }
}

/COPYRIGHT/ {
    printf ("	/* %s */\n", $0) ;
}

/STARTCHAR/ {
    if ((length ($2) > 1) && (substr ($2, 0, 1) == "C")) {
        character = $2
    } else {
        character = sprintf ("\"%s\"", $2)
    }
}

/ENCODING/ {
    encoding = $2
}

/BITMAP/,/ENDCHAR/ {
    if ($1 == "BITMAP") {
        printf ("   ") ;
        pixel = 0
    } else if ($1 == "ENDCHAR") {
        printf ("\n") ;
    } else {
        printf (" 0x%s,", substr ($1, 0, 2)) ;
        pixel = pixel + 1 ;
        if (pixel == 8) {
            printf ("	/* %s (%s) */\n   ", character, encoding) ;
        }
    }
}
