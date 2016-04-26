/* $Id: hash_util.c,v 1.19 2004/02/16 20:54:12 alex Exp $ */
/*******************************************************************************

File:

    hash_util.c

    Hash Search Utilities


Author:    Alex Measday


Purpose:

    These utilities provide a means of building hash tables and performing
    hash searches.

    The classic representation of hash tables is used for these hash tables.
    An array of buckets is created by hashCreate(), sized to the first prime
    number M that is larger than the expected maximum number of elements in
    the table.  Key-value pairs are then added to the table by hashAdd().
    A character string key is "folded" into an integer and divided by the
    prime number M to produce an index into the array of buckets; the key-value
    pair is then stored in the indicated bucket.  If multiple key-value pairs
    map into the same bucket (AKA a collision), they are chained together by a
    linked list attached to the bucket.

    Building a hash table using these functions is very simple.  First, create
    an empty hash table:

        #include  "hash_util.h"		-- Hash table definitions.
        #define  NUM_ITEMS  500
        HashTable  table ;
        ...
        hashCreate (NUM_ITEMS, &table) ;

    The first argument to hashCreate() is the expected number of items in
    the table; the table will handle more, albeit with slower lookup times.

    Key-value pairs are added to the table with hashAdd():

        hashAdd (table, "<key>", (void *) value) ;

    Keys are null-terminated characters strings and values must be cast as
    (VOID *) pointers.  If the key is already in the table, its old value
    is replaced with the new value.

    Looking up the value of a key is done with hashSearch():

        void  *value ;
        ...
        if (hashSearch (table, "<key>", &value))
            ... found ...
        else
            ... not found ...

    The value is returned as a (VOID *) pointer, which the caller must then
    cast back to the original type.

    Key-value pairs can be individually deleted from a hash table:

        hashDelete (table, "<key>") ;

    or the entire table can be destroyed:

        hashDestroy (table) ;

    The HASH_UTIL group of hash table functions offer several advantages
    over the Standard C Library hashing functions (HCREATE(3), HDESTROY(3),
    and HSEARCH(3)).  First, the HASH_UTIL functions are easier to use: the
    multi-purpose functionality of HSEARCH(3) is broken up into hashAdd()
    and hashSearch(), etc.  Second, the HASH_UTIL functions allow for more
    than one hash table in a program.


Procedures:

    hashAdd() - adds a key-data pair to a hash table.
    hashCount() - returns the number of key-data pairs in a hash table.
    hashCreate() - creates an empty hash table.
    hashDelete() - deletes a key-data pair from a hash table.
    hashDestroy() - deletes a hash table.
    hashDump() - dumps a hash table.
    hashFind() - finds a data value by name in a hash table.
    hashGet() - gets a key by index in a hash table.
    hashSearch() - locates a key in a hash table and returns the data value
        associated with the key.
    hashStatistics() - displays various statistics for a hash table.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <math.h>			/* Math library definitions. */
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "hash_util.h"			/* Hash table definitions. */


/*******************************************************************************
    Hash Table Data Structures.
*******************************************************************************/

typedef  struct  HashItem {
    char  *key ;			/* Item key. */
    void  *value ;			/* Item value. */
    struct  HashItem  *next ;		/* Pointer to next item in list. */
}  HashItem ;

typedef  struct  _HashTable {
    int  totalItems ;			/* Total # of items in table. */
    int  maxChains ;			/* Maximum number of entries N in table. */
    int  numChains ;			/* Actual number of non-empty entries. */
    int  longestChain ;			/* Records length of longest chain. */
    HashItem  **chain ;			/* Array of N pointers to item chains. */
    int  *numItems ;			/* Array: # of items in each chain. */
}  _HashTable ;


int  hash_util_debug = 0 ;		/* Global debug switch (1/0 = yes/no). */
#undef  I_DEFAULT_GUARD
#define  I_DEFAULT_GUARD  hash_util_debug


/*******************************************************************************
    Private Functions
*******************************************************************************/

static  int  hashKey (
#    if PROTOTYPES
        const  char  *key,
        int  tableSize
#    endif
    ) ;

static  int  hashPrime (
#    if PROTOTYPES
        int  number
#    endif
    ) ;

/*******************************************************************************

Procedure:

    hashAdd ()


Purpose:

    Function hashAdd() adds a key-value pair to a hash table.  If the key is
    already present in the table, its old value is replaced by the new value.
    The table must have already been created by hashCreate().


    Invocation:

        status = hashAdd (table, key, data) ;

    where

        <table>
            is the hash table handle returned by hashCreate().
        <key>
            is the key for the item being entered in the table.
        <data>
            is the data to be associated with the key.  This argument is a
            (VOID *) pointer; whatever data or pointer to data being passed
            in for this argument should be cast to (VOID *).
        <status>
            returns the status of adding the key to the hash table, zero if
            no errors occurred and ERRNO otherwise.

*******************************************************************************/


int  hashAdd (

#    if PROTOTYPES
        HashTable  table,
        const  char  *key,
        const  void  *data)
#    else
        table, key, data)

        HashTable  table ;
        char  *key ;
        void  *data ;
#    endif

{    /* Local variables. */
    HashItem  *item, *prev ;
    int  comparison, index ;





    if (table == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(hashAdd) Hash table not created yet.\n") ;
        return (errno) ;
    }


/* If the key is already in the hash table, then replace its data value. */

    index = hashKey (key, table->maxChains) ;
    comparison = -1 ;  prev = (HashItem *) NULL ;
    for (item = table->chain[index] ;  item != NULL ;  item = item->next) {
        comparison = strcmp (item->key, key) ;
        if (comparison >= 0)  break ;
        prev = item ;
    }

    if (comparison == 0) {
        item->value = (void *) data ;
        LGI "(hashAdd) Replaced \"%s\":%p (%p) in table %p[%d].\n",
            key, data, (void *) item, (void *) table, index) ;
        return (0) ;
    }


/* Add a brand new item to the hash table: allocate an ITEM node for the item,
   fill in the fields, and link the new node into the chain of items. */

    item = (HashItem *) malloc (sizeof (HashItem)) ;	/* Allocate item node. */
    if (item == NULL) {
        LGE "(hashAdd) Error allocating item for \"%s\":%p.\nmalloc: ",
            key, data) ;
        return (errno) ;
    }

    item->key = strdup (key) ;			/* Fill in the item node. */
    if (item->key == NULL) {
        LGE "(hashAdd) Error duplicating key \"%s\".\nstrdup: ", key) ;
        free ((char *) item) ;
        return (errno) ;
    }
    item->value = (void *) data ;

    if (prev == NULL) {				/* Link in at head of list. */
        item->next = table->chain[index] ;
        if (item->next == NULL)  table->numChains++ ;
        table->chain[index] = item ;
    } else {					/* Link in further down the list. */
        item->next = prev->next ;
        prev->next = item ;
    }

    table->numItems[index]++ ;
    table->totalItems++ ;

    LGI "(hashAdd) Added \"%s\":%p (%p) to table %p[%d].\n",
        key, data, (void *) item, (void *) table, index) ;


/* For statistical purposes, measure the length of the chain and, if necessary,
   update the LONGEST_CHAIN value for the hash table. */

    comparison = 0 ;
    for (item = table->chain[index] ;  item != NULL ;  item = item->next) {
        comparison++ ;
    }
    if (table->longestChain < comparison)
        table->longestChain = comparison ;


    return (0) ;

}

/*******************************************************************************

Procedure:

    hashCount ()

    Count the Number of Key/Data Pairs in a Table.


Purpose:

    Function hashCount() returns the number of key/data pairs in a hash table.


    Invocation:

        count = hashCount (table) ;

    where

        <table>	- I
            is the hash table handle returned by hashCreate().
        <count>	- O
            returns the number of key/data pairs in the hash table.

*******************************************************************************/


int  hashCount (

#    if PROTOTYPES
        HashTable  table)
#    else
        table)

        HashTable  table ;
#    endif

{

    return ((table == NULL) ? 0 : table->totalItems) ;

}

/*******************************************************************************

Procedure:

    hashCreate ()


Purpose:

    Function hashCreate() creates an empty hash table.  hashAdd() can then be
    called to add entries to the table.  The number of buckets in the table
    is equal to the first prime number larger than the expected maximum number
    of elements in the table.


    Invocation:

        status = hashCreate (maxEntries, &table) ;

    where

        <maxEntries>
            is the maximum number of entries expected in the table.
        <table>
            returns a handle for the new hash table.  This handle is
            used for accessing the table in subsequent HASH_UTIL calls.
        <status>
            returns the status of creating the hash table, zero if no errors
            occurred and ERRNO otherwise.

*******************************************************************************/


int  hashCreate (

#    if PROTOTYPES
        int  maxEntries,
        HashTable  *table)
#    else
        maxEntries, table)

        int  maxEntries ;
        HashTable  *table ;
#    endif

{    /* Local variables. */
    int  i, prime ;




/* Find the first prime number larger than the expected number of entries
   in the table. */

    prime = (maxEntries % 2) ? maxEntries : maxEntries + 1 ;
    for ( ; ; ) {			/* Check odd numbers only. */
        if (hashPrime (prime))  break ;
        prime += 2 ;
    }

/* Create and initialize the hash table. */

    *table = (HashTable) malloc (sizeof (_HashTable)) ;
    if (*table == NULL) {
        LGE "(hashCreate) Error allocating hash table header.\nmalloc: ") ;
        return (errno) ;
    }

    (*table)->totalItems = 0 ;
    (*table)->maxChains = prime ;
    (*table)->numChains = 0 ;		/* Number of non-empty chains. */
    (*table)->longestChain = 0 ;	/* Length of longest chain. */

/* Allocate the array of chains. */

    (*table)->chain = (HashItem **) calloc (prime, sizeof (HashItem *)) ;
    if ((*table)->chain == NULL) {
        LGE "(hashCreate) Error allocating %d-element array of chains.\ncalloc: ",
            prime) ;
        return (errno) ;
    }
    for (i = 0 ;  i < prime ;  i++)
        (*table)->chain[i] = NULL ;

/* Allocate the parallel array of chain lengths. */

    (*table)->numItems = (int *) calloc (prime, sizeof (int)) ;
    if ((*table)->numItems == NULL) {
        LGE "(hashCreate) Error allocating %d-element array of chain lengths.\ncalloc: ",
            prime) ;
        return (errno) ;
    }
    for (i = 0 ;  i < prime ;  i++)
        (*table)->numItems[i] = 0 ;

    LGI "(hashCreate) Created hash table %p of %d elements.\n",
        (void *) *table, prime) ;

    return (0) ;

}

/*******************************************************************************

Procedure:

    hashDelete ()


Purpose:

    Function hashDelete() deletes a key-data entry from a hash table.  The
    table must have already been created by hashCreate() and the key-data
    entry added to the table by hashAdd().


    Invocation:

        status = hashDelete (table, key) ;

    where

        <table>
            is the hash table handle returned by hashCreate().
        <key>
            is the key for the item being deleted from the table.
        <status>
            returns the status of deleting the key from the hash table, zero
            if no errors occurred and ERRNO otherwise.

*******************************************************************************/


int  hashDelete (

#    if PROTOTYPES
        HashTable  table,
        const  char  *key)
#    else
        table, key)

        HashTable  table ;
        char  *key ;
#    endif

{    /* Local variables. */
    HashItem  *item, *prev ;
    int  index ;





    if (table == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(hashDelete) Hash table not created yet.\n") ;
        return (errno) ;
    }

/* Locate the key's entry in the hash table. */

    index = hashKey (key, table->maxChains) ;
    prev = (HashItem *) NULL ;
    for (item = table->chain[index] ;  item != NULL ;  item = item->next) {
        if (strcmp (item->key, key) == 0)  break ;
        prev = item ;
    }

/* Unlink the entry from the hash table and free it. */

    if (item == NULL) {
        LGI "(hashDelete) Key \"%s\" not found in table %p.\n",
            key, (void *) table) ;
        return (-2) ;
    } else {
        if (prev == NULL)
            table->chain[index] = item->next ;
        else
            prev->next = item->next ;
        table->numItems[index]-- ;
        table->totalItems-- ;
    }

    LGI "(hashDelete) Deleted \"%s\":%p from table %p.\n",
        item->key, item->value, (void *) table) ;

    free (item->key) ;			/* Free item key. */
    free ((char *) item) ;		/* Free the item. */

    return (0) ;

}

/*******************************************************************************

Procedure:

    hashDestroy ()


Purpose:

    Function hashDestroy() deletes a hash table.


    Invocation:

        status = hashDestroy (table) ;

    where

        <table>
            is the hash table handle returned by hashCreate().
        <status>
            returns the status of deleting the hash table, zero if no errors
            occurred and ERRNO otherwise.

*******************************************************************************/


int  hashDestroy (

#    if PROTOTYPES
        HashTable  table)
#    else
        table)

        HashTable  table ;
#    endif

{    /* Local variables. */
    int  i ;
    HashItem  *item, *next ;




    LGI "(hashDestroy) Deleting hash table %p.\n", (void *) table) ;

    if (table == NULL)  return (0) ;

    for (i = 0 ;  i < table->maxChains ;  i++) {
        for (item = table->chain[i] ;  item != NULL ;  item = next) {
            next = item->next ;
            free (item->key) ;			/* Free item key. */
            free ((char *) item) ;		/* Free the item. */
        }
    }

/* Free the hash table. */

    if (table->chain != NULL)  free (table->chain) ;
    if (table->numItems != NULL)  free (table->numItems) ;
    free (table) ;

    return (0) ;

}

/*******************************************************************************

Procedure:

    hashDump ()


Purpose:

    Function hashDump() dumps a hash table to the specified output file.


    Invocation:

        status = hashDump (outfile, header, table) ;

    where

        <outfile>
            is the UNIX file descriptor (FILE *) for the output file.
        <header>
            is a text string to be output as a header.  The string is
            actually used as the format string in an FPRINTF statement,
            so you need to include any newlines ("\n"), etc. that you
            need.  This argument can be NULL.
        <table>
            is the hash table handle returned by hashCreate().
        <status>
            returns zero.

*******************************************************************************/


int  hashDump (

#    if PROTOTYPES
        FILE  *outfile,
        const  char  *header,
        HashTable  table)
#    else
        outfile, header, table)

        FILE  *outfile ;
        char  *header ;
        HashTable  table ;
#    endif

{    /* Local variables. */
    int  i ;
    HashItem  *item ;




    if (header != NULL)  fprintf (outfile, header) ;

    if (table == NULL) {
        fprintf (outfile, "-- Null Hash Table --\n") ;
        return (0) ;
    }

#ifdef HASH_STATISTICS
    hashStatistics (outfile, table) ;
    fprintf (outfile, "\n") ;
#endif

    for (i = 0 ;  i < table->maxChains ;  i++) {
        item = table->chain[i] ;
        if (item != NULL) {
            fprintf (outfile, "Bucket %d:\n", i) ;
            while (item != NULL) {
                fprintf (outfile, "    Value: %p    Key: \"%s\"\n",
                         item->value, item->key) ;
                item = item->next ;
            }
        }
    }

    return (0) ;

}

/*******************************************************************************

Procedure:

    hashFind ()

    Find a Data Value by Name in a Table.


Purpose:

    Function hashFind() looks up a key in a hash table and returns the
    data associated with that key.

        NOTE: Use hashSearch() if a key may have a NULL data value.


    Invocation:

        data = hashFind (table, key) ;

    where

        <table>	- I
            is the hash table handle returned by hashCreate().
        <key>	- I
            is the key for the item being searched in the table.
        <data>	- O
            returns the data associated with the key.  The value returned is
            a (VOID *) pointer; this pointer can be cast back to whatever data
            or pointer to data was stored by hashAdd().  NULL is returned if
            the key is not found in the hash table.

*******************************************************************************/


void  *hashFind (

#    if PROTOTYPES
        HashTable  table,
        const  char  *key)
#    else
        table, key)

        HashTable  table ;
        const  char  *key ;
#    endif

{    /* Local variables. */
    void  *data ;


    if (hashSearch (table, key, &data))
        return (data) ;					/* Found. */
    else
        return (NULL) ;					/* Not found. */

}

/*******************************************************************************

Procedure:

    hashGet ()

    Find a Key by Index in a Table.


Purpose:

    Function hashGet() returns the I-th key in a hash table and is useful for
    iterating through the key/value mappings in the hash table.  The ordering
    of keys is dependent on their location in the hash table and this ordering
    should not be relied upon by the application.


    Invocation:

        key = hashGet (table, index, &data) ;

    where

        <table>	- I
            is the hash table handle returned by hashCreate().
        <index>	- I
            is the index (0..N-1) of the desired key.
        <data>	- O
            optionally returns the data associated with the key.  If this
            argument is NULL, the data value is not returned.  Otherwise,
            the value returned is a (VOID *) pointer; this pointer can be
            cast back to whatever data or pointer to data was stored by
            hashAdd().
        <key>	- O
            returns the indexed key in the table; NULL is returned if the
            index is out of bounds.

*******************************************************************************/


const  char  *hashGet (

#    if PROTOTYPES
        HashTable  table,
        int  index,
        void  **data)
#    else
        table, index, data)

        HashTable  table ;
        int  index ;
        void  **data ;
#    endif

{    /* Local variables. */
    int  i, totalItems ;
    HashItem  *item ;



/* Locate the hash chain containing the indexed key. */

    totalItems = 0 ;
    for (i = 0 ;  i < table->maxChains ;  i++) {
        totalItems += table->numItems[i] ;
        if (totalItems > index)  break ;
    }

    if ((index < 0) || (i >= table->maxChains))
        return (NULL) ;				/* Out-of-bounds index. */

/* Traverse the chain until the indexed key is reached. */

    item = table->chain[i] ;
    totalItems -= table->numItems[i] ;
    while (totalItems++ < index) {
        item = item->next ;
    }

/* Return the key and data value to the caller. */

    if (data != NULL)  *data = item->value ;

    return (item->key) ;

}

/*******************************************************************************

Procedure:

    hashKey ()


Purpose:

    Function hashKey() converts a character string key to an integer index
    into a hash table.  The conversion takes place in two steps: (i) "fold"
    the character string key into an integer, and (ii) divide that integer
    by the number of buckets in the table (a prime number computed by
    hashCreate()).


    Invocation:

        index = hashKey (key, tableSize) ;

    where

        <key>
            is a character string key.
        <tableSize>
            is the size M of the table being hashed.
        <index>
            returns the index, [0..M-1], in the table of where the key
            can be found.

*******************************************************************************/


static  int  hashKey (

#    if PROTOTYPES
        const  char  *key,
        int  tableSize)
#    else
        key, tableSize)

        char  *key ;
        int  tableSize ;
#    endif

{    /* Local variables. */
    const  char  *s ;
    unsigned  int  value ;



    if (tableSize == 0)  return (0) ;	/* Empty table? */

/* Fold the character string key into an integer number.  I found the algorithm
   via NIST ( http://www.nist.gov/dads/HTML/hash.html ); the algorithm is from
   the HANDBOOK OF ALGORITHMS AND DATA STRUCTURES by Gaston H. Gonnet and
   Ricardo Baeza-Yates ( http://www.dcc.uchile.cl/~rbaeza/handbook/ ).  The
   on-line version of the handbook doesn't explain the code snippet (#331),
   but I imagine the algorithm is tuned for 7-bit ASCII: a 7-bit left shift is
   equivalent to multiplying by 128, and 131 is the firt prime number larger
   than 128.  Regardless of my imagination, the hashing function performed
   well both with short text keys that varied in all positions and with long
   text keys that varied only in the final position(s). */

    for (s = key, value = 0 ;  *s != '\0' ;  s++) {
        value = (value * 131) + *s ;
    }

    return (value % tableSize) ;	/* Return index [0..M-1] into table. */

}

/*******************************************************************************

Procedure:

    hashPrime ()


Purpose:

    Function hashPrime() determines if a number is a prime number.


    Invocation:

        isPrime = hashPrime (number) ;

    where

        <number>
            is the number to be checked for "prime"-ness.
        <isPrime>
            return TRUE (non-zero) if NUMBER is prime and FALSE (zero) if
            NUMBER is not prime.

*******************************************************************************/


static  int  hashPrime (

#    if PROTOTYPES
        int  number)
#    else
        number)

        int  number ;
#    endif

{    /* Local variables. */
    int  divisor ;



    if (number < 0)  number = -number ;
    if (number < 4)  return (1) ;	/* 0, 1, 2, and 3 are prime. */

/* Check for possible divisors.  The "divisor > dividend" test is similar
   to checking 2 .. sqrt(N) as possible divisors, but avoids the need for
   linking to the math library. */

    for (divisor = 2 ;  ;  divisor++) {
        if ((number % divisor) == 0)
            return (0) ;		/* Not prime - divisor found. */
        if (divisor > (number / divisor))
            return (1) ;		/* Prime - no divisors found. */
    }

}

/*******************************************************************************

Procedure:

    hashSearch ()


Purpose:

    Function hashSearch() looks up a key in a hash table and returns the
    data associated with that key.  The hash table must be created using
    hashCreate() and entries must be added to the table using hashAdd().


    Invocation:

        found = hashSearch (table, key, &data) ;

    where

        <table>
            is the hash table handle returned by hashCreate().
        <key>
            is the key for the item being searched in the table.
        <data>
            returns the data associated with the key.  The value returned is
            a (VOID *) pointer; this pointer can be cast back to whatever data
            or pointer to data was stored by hashAdd().
        <found>
            returns TRUE (non-zero) if the key was found in the hash table;
            FALSE (zero) is returned if the key was not found.

*******************************************************************************/


int  hashSearch (

#    if PROTOTYPES
        HashTable  table,
        const  char  *key,
        void  **data)
#    else
        table, key, data)

        HashTable  table ;
        char  *key ;
        void  **data ;
#    endif

{    /* Local variables. */
    HashItem  *item ;
    int  comparison, index ;



/* Lookup the item in the hash table. */

    index = hashKey (key, table->maxChains) ;
    comparison = -1 ;
    for (item = table->chain[index] ;  item != NULL ;  item = item->next) {
        comparison = strcmp (item->key, key) ;
        if (comparison >= 0)  break ;
    }

/* If found, return the item's data value to the calling routine. */

    if (comparison == 0) {
        if (data != NULL)  *data = (void *) item->value ;
        LGI "(hashSearch) \"%s\":%p found in table %p.\n",
            key, item->value, (void *) table) ;
        return (-1) ;
    } else {
        if (data != NULL)  *data = NULL ;
        LGI "(hashSearch) Key \"%s\" not found in table %p.\n",
            key, (void *) table) ;
        return (0) ;
    }

}

#ifdef HASH_STATISTICS
/*******************************************************************************

Procedure:

    hashStatistics ()


Purpose:

    Function hashStatistics() outputs various statistical measurements for
    a hash table.


    Invocation:

        status = hashStatistics (outfile, table) ;

    where

        <outfile>
            is the UNIX file descriptor (FILE *) for the output file.
        <table>
            is the hash table handle returned by hashCreate().
        <status>
            returns zero.

*******************************************************************************/


int  hashStatistics (

#    if PROTOTYPES
        FILE  *outfile,
        HashTable  table)
#    else
        outfile, table)

        FILE  *outfile ;
        HashTable  table ;
#    endif

{    /* Local variables. */
    int  count, *histogram, i, longestChain, maxChains, numChains ;
    long  sum, sumOfSquares ;
    HashItem  *item ;




    if (table == NULL) {
        fprintf (outfile, "-- Null Hash Table --\n") ;
        return (0) ;
    }

    maxChains = table->maxChains ;
    numChains = table->numChains ;
    longestChain = table->longestChain ;

    fprintf (outfile, "There are %d empty buckets, %d non-empty buckets,\nand %d items in the longest chain.\n\n",
             maxChains - numChains, numChains, longestChain) ;

    histogram = (int *) malloc ((longestChain + 1) * sizeof (int)) ;
    if (histogram == NULL) {
        LGE "(hashStatistics) Error allocating memory for histogram.\nmalloc: ") ;
        return (errno) ;
    }

    for (count = 0 ;  count <= longestChain ;  count++)
        histogram[count] = 0 ;

    for (i = 0 ;  i < maxChains ;  i++) {
        item = table->chain[i] ;
        for (count = 0 ;  item != NULL ;  count++)
            item = item->next ;
        histogram[count]++ ;
    }

    for (count = 1, sum = sumOfSquares = 0 ;
         count <= longestChain ;  count++) {
        fprintf (outfile, "Buckets of length %d: %d\n", count, histogram[count]) ;
        sum = sum  +  histogram[count] * count ;
        sumOfSquares = sumOfSquares  +  histogram[count] * count * count ;
    }

    fprintf (outfile, "\nMean bucket length = %G\n",
             (double) sum / (double) numChains) ;
    fprintf (outfile, "\nStandard deviation = %G\n",
             sqrt ((((double) numChains * sumOfSquares) - ((double) sum * sum)) /
                   ((double) numChains * (numChains - 1)))) ;

    free ((char *) histogram) ;

    return (0) ;

}
#endif /* HASH_STATISTICS */

#ifdef  TEST

/*******************************************************************************

    Program to test the HASH_UTIL routines.  Compile as follows:

        % cc -g -DTEST hash_util.c -I<... includes ...>

    Invocation:

        % a.out [ <num_entries> ]

*******************************************************************************/


int  main (argc, argv)

    int  argc ;
    char  *argv[] ;

{    /* Local variables. */
    char  text[16] ;
    HashTable  table ;
    int  i, maxNumEntries ;
    void  *data ;




    maxNumEntries = (argc > 1) ? atoi (argv[1]) : 100 ;

/* Create an empty hash table. */

    if (hashCreate (maxNumEntries, &table)) {
        LGE "Error creating table.\nhashCreate: ") ;
        exit (errno) ;
    }

/* Add "SYM_n" symbols to the table. */

    for (i = 0 ;  i < maxNumEntries ;  i++) {
        sprintf (text, "SYM_%d", i) ;
        if (hashAdd (table, text, (void *) i)) {
            LGE "Error adding entry %d to the table.\nhashAdd: ", i) ;
            exit (errno) ;
        }
    }

/* Verify that the symbols were entered correctly and with the correct value. */

    for (i = 0 ;  i < maxNumEntries ;  i++) {
        sprintf (text, "SYM_%d", i) ;
        if (!hashSearch (table, text, &data) || ((int) data != i)) {
            LGE "Error looking up entry %d in the table.\nhashSearch: ", i) ;
            exit (errno) ;
        }
    }

/* Dump the hash table. */

    hashDump (stdout, "\n", table) ;

/* Delete the hash table. */

    if (hashDestroy (table)) {
        LGE "Error deleting the hash table.\nhashDestroy: ") ;
        exit (errno) ;
    }

}
#endif /* TEST */
