// bslmf_switch.t.cpp                                                 -*-C++-*-

#include <bslmf_switch.h>

#include <bslmf_nil.h>

#include <bsls_bsltestutil.h>

#include <stdio.h>   // 'printf'
#include <stdlib.h>  // 'atoi'
#include <string.h>

using namespace BloombergLP;

//=============================================================================
//                             TEST PLAN
//-----------------------------------------------------------------------------
// The component under test is a meta-function returning a 'TYPE' as a function
// of its template arguments.  As such, the concerns are limited to the
// correctness of the return "value", which is established by using overload
// resolution as the mechanism to discriminate among types.
//
// [ 2] bslmf::SwitchN<SELECTOR,T0,. . .,T{N-1}>::Type
// [ 1] bslmf::Switch<SELECTOR,T0,T1,T2,T3,T4,T5,T6,T7,T8,T9>::Type
//-----------------------------------------------------------------------------
// [ 1] FUNCTIONALITY TEST (CLASS 'bslmf::Switch')
// [ 2] CLASSES 'bslmf::SwitchN'

//=============================================================================
//                      STANDARD BDE ASSERT TEST MACRO
//-----------------------------------------------------------------------------

// NOTE: THIS IS A LOW-LEVEL COMPONENT AND MAY NOT USE ANY C++ LIBRARY
// FUNCTIONS, INCLUDING IOSTREAMS.

static int testStatus = 0;

void aSsErT(bool b, const char *s, int i)
{
    if (b) {
        printf("Error " __FILE__ "(%d): %s    (failed)\n", i, s);
        if (testStatus >= 0 && testStatus <= 100) ++testStatus;
    }
}

//=============================================================================
//                      STANDARD BDE TEST DRIVER MACROS
//-----------------------------------------------------------------------------

#define ASSERT       BSLS_BSLTESTUTIL_ASSERT
#define LOOP_ASSERT  BSLS_BSLTESTUTIL_LOOP_ASSERT
#define LOOP0_ASSERT BSLS_BSLTESTUTIL_LOOP0_ASSERT
#define LOOP1_ASSERT BSLS_BSLTESTUTIL_LOOP1_ASSERT
#define LOOP2_ASSERT BSLS_BSLTESTUTIL_LOOP2_ASSERT
#define LOOP3_ASSERT BSLS_BSLTESTUTIL_LOOP3_ASSERT
#define LOOP4_ASSERT BSLS_BSLTESTUTIL_LOOP4_ASSERT
#define LOOP5_ASSERT BSLS_BSLTESTUTIL_LOOP5_ASSERT
#define LOOP6_ASSERT BSLS_BSLTESTUTIL_LOOP6_ASSERT
#define ASSERTV      BSLS_BSLTESTUTIL_ASSERTV

#define Q   BSLS_BSLTESTUTIL_Q   // Quote identifier literally.
#define P   BSLS_BSLTESTUTIL_P   // Print identifier and value.
#define P_  BSLS_BSLTESTUTIL_P_  // P(X) without '\n'.
#define T_  BSLS_BSLTESTUTIL_T_  // Print a tab (w/o newline).
#define L_  BSLS_BSLTESTUTIL_L_  // current Line number

//=============================================================================
//                  GLOBAL TYPEDEFS/CONSTANTS FOR TESTING
//-----------------------------------------------------------------------------

enum { VERBOSE_ARG_NUM = 2, VERY_VERBOSE_ARG_NUM, VERY_VERY_VERBOSE_ARG_NUM };

static int verbose = 0;
static int veryVerbose = 0;
static int veryVeryVerbose = 0;

//=============================================================================
//                  GLOBAL HELPER FUNCTIONS FOR TESTING
//-----------------------------------------------------------------------------

struct A { };
struct B { };
struct C { };
struct D { };
struct E { };
struct F { };
struct G { };
struct H { };
struct I { };
struct J { };

char f(A) { return 'A'; }
char f(B) { return 'B'; }
char f(C) { return 'C'; }
char f(D) { return 'D'; }
char f(E) { return 'E'; }
char f(F) { return 'F'; }
char f(G) { return 'G'; }
char f(H) { return 'H'; }
char f(I) { return 'I'; }
char f(J) { return 'J'; }
char f(bslmf::Nil) { return '0'; }

//=============================================================================
//                  CLASSES FOR TESTING USAGE EXAMPLES
//-----------------------------------------------------------------------------

// Assume an external server API for storing and retrieving data:
//..
    class data_Server {
        // Dummy implementation of data server

        int d_data;

      public:
        void store(char  data) { d_data = data | 0Xefface00; }
        void store(short data) { d_data = data | 0Xdead0000; }
        void store(int   data) { d_data = data; }

        void retrieve(char  *data) { *data = d_data & 0x000000ff; }
        void retrieve(short *data) { *data = d_data & 0x0000ffff; }
        void retrieve(int   *data) { *data = d_data; }
    };
//..
// In our application, we need some very small (1, 2, and 4-byte),
// special-purpose string types, so we create the following 'ShortString'
// class template:
//..
    template <int LEN>
    class ShortString {
        // Store a short, fixed-length string.

        char d_buffer[LEN];

      public:
        ShortString(const char *s = "") { strncpy(d_buffer, s, LEN); }
            // Construct a 'ShortString' from a NTCS.

        void retrieve(data_Server *server);
            // Retrieve this string from a data server.

        void store(data_Server *server) const;
            // Store this string to a data server.

        char operator[](int n) const { return d_buffer[n]; }
            // Return the nth byte in this string.
    };

    template <int LEN>
    bool operator==(const ShortString<LEN>& lhs, const ShortString<LEN>& rhs)
        // Return true if a 'lhs' is equal to 'rhs'
    {
        return 0 == memcmp(&lhs, &rhs, LEN);
    }

    template <int LEN>
    bool operator==(const ShortString<LEN>& lhs, const char *rhs)
        // Return true if a 'ShortString' 'lhs' is equal to a NTCS 'rhs'.
    {
        int i;
        for (i = 0; LEN > i && lhs[i]; ++i) {
            if (lhs[i] != rhs[i]) {
                return false;
            }
        }

        return ('\0' == rhs[i]);
    }
//..
// We would like to store our short strings in the data server, but the data
// server only handles 'char', 'short' and 'int' types.  Since our strings fit
// into these simple types, we can transform 'ShortString' into these integral
// types when calling 'store' and 'retrieve', using 'bslmf::Switch' to choose
// which integral type to use for each 'ShortString' type:
//..
    template <int LEN>
    void ShortString<LEN>::retrieve(data_Server *server)
    {
        // 'transferType will be 'char' if 'LEN' is 1, 'short' if 'LEN' is 2,
        // and 'int' if 'LEN' 4.  Will choose 'void' and thus not compile if
        // 'LEN' is 0 or 3.
        typedef typename
          bslmf::Switch5<LEN, void, char, short, void, int>::Type transferType;

        transferType x = 0;
        server->retrieve(&x);
        memcpy(d_buffer, &x, LEN);
    }

    template <int LEN>
    void ShortString<LEN>::store(data_Server *server) const
    {
        // 'transferType will be 'char' if 'LEN' is 1, 'short' if 'LEN' is 2,
        // and 'int' if 'LEN' 4.  Will choose 'void' and thus not compile if
        // 'LEN' is 0 or 3.
        typedef typename
          bslmf::Switch5<LEN, void, char, short, void, int>::Type transferType;

        transferType x = 0;
        memcpy(&x, d_buffer, LEN);
        server->store(x);
    }
//..
// In our main program, we first assert our basic assumptions, then we
// store and retrieve strings using our 'ShortString' template.
//..
    int usageExample()
    {
        ASSERT(2 == sizeof(short));
        ASSERT(4 == sizeof(int));

        data_Server server;

        ShortString<1> a("A");
        ShortString<1> b("B");
        ASSERT(a == "A");
        ASSERT(b == "B");
        ASSERT(! (a == b));

        a.store(&server);
        b.retrieve(&server);
        ASSERT(a == "A");
        ASSERT(b == "A");
        ASSERT(a == b);

        ShortString<2> cd("CD");
        ShortString<2> ef("EF");
        ASSERT(cd == "CD");
        ASSERT(ef == "EF");
        ASSERT(! (cd == ef));

        cd.store(&server);
        ef.retrieve(&server);
        ASSERT(cd == "CD");
        ASSERT(ef == "CD");
        ASSERT(cd == ef);

        ShortString<4> ghij("GHIJ");
        ShortString<4> klmn("KLMN");
        ASSERT(ghij == "GHIJ");
        ASSERT(klmn == "KLMN");
        ASSERT(! (ghij == klmn));

        ghij.store(&server);
        klmn.retrieve(&server);
        ASSERT(ghij == "GHIJ");
        ASSERT(klmn == "GHIJ");
        ASSERT(ghij == klmn);

        return 0;
    }
//..

//=============================================================================
//                              MAIN PROGRAM
//-----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    int test = argc > 1 ? atoi(argv[1]) : 0;
    verbose = argc > 2;
    veryVerbose = argc > 3;
    veryVeryVerbose = argc > 4;

    setbuf(stdout, 0);    // Use unbuffered output

    printf("TEST " __FILE__ " CASE %d\n", test);

    switch (test) { case 0:  // Zero is always the leading case.
      case 3: {
        // --------------------------------------------------------------------
        // TESTING USAGE EXAMPLE
        //   The usage example provided in the component header file must
        //   compile, link, and run on all platforms as shown.
        //
        // Plan:
        //   Incorporate usage example from header into driver, remove leading
        //   comment characters, and replace 'assert' with 'ASSERT'.
        //
        // Testing:
        //   USAGE EXAMPLE
        // --------------------------------------------------------------------

        if (verbose) printf("\nUSAGE EXAMPLE"
                                 "\n=============\n");

        usageExample();

      } break;
      case 2: {
        // --------------------------------------------------------------------
        // TESTING CLASSES 'bslmf::SwitchN'
        //
        // Concerns:
        //   Returns the right type for 'SELECTOR' values 0-N.
        //   Returns 'bslmf::Nil' for 'SELECTOR' < 0 or N < 'SELECTOR'
        //
        // Plan:  For each 'N' between 2 and 9:
        //   Instantiate with N different types each value in the range 0-(N-1)
        //       as 'SELECTOR'.  Call an 'f' function on the result to verify
        //       the type.  Instantiate with 'SELECTOR' out of range 0-(N-1)
        //       and verify that 'f' returns '0'.
        //   Note that, compared to case 1, we do not worry about defaulted nor
        //   repeated template arguments.
        //
        // Testing:
        //     bslmf::SwitchN<SELECTOR,T0,. . .,T{N-1}>::Type
        // --------------------------------------------------------------------

        if (verbose) printf("\nFUNCTIONALITY TEST"
                                 "\n==================\n");

        if (verbose) printf("\tTesting 'bslmf_Switch2.\n");
        {
            if (verbose) printf("Testing SELECTOR in range\n");
            ASSERT('A' == f(bslmf::Switch2<0,A,B>::Type()));
            ASSERT('B' == f(bslmf::Switch2<1,A,B>::Type()));

            if (verbose) printf("Testing SELECTOR out of range\n");
            ASSERT('0' == f(bslmf::Switch2<2,A,B>::Type()));
            ASSERT('0' == f(bslmf::Switch2<10,A,B>::Type()));
            ASSERT('0' == f(bslmf::Switch2<-1,A,B>::Type()));
        }

        if (verbose) printf("\tTesting 'bslmf_Switch3.\n");
        {
            if (verbose) printf("Testing SELECTOR in range\n");
            ASSERT('A' == f(bslmf::Switch3<0,A,B,C>::Type()));
            ASSERT('B' == f(bslmf::Switch3<1,A,B,C>::Type()));
            ASSERT('C' == f(bslmf::Switch3<2,A,B,C>::Type()));

            if (verbose) printf("Testing SELECTOR out of range\n");
            ASSERT('0' == f(bslmf::Switch3<3,A,B,C>::Type()));
            ASSERT('0' == f(bslmf::Switch3<10,A,B,C>::Type()));
            ASSERT('0' == f(bslmf::Switch3<-1,A,B,C>::Type()));
        }

        if (verbose) printf("\tTesting 'bslmf_Switch4.\n");
        {
            if (verbose) printf("Testing SELECTOR in range\n");
            ASSERT('A' == f(bslmf::Switch4<0,A,B,C,D>::Type()));
            ASSERT('B' == f(bslmf::Switch4<1,A,B,C,D>::Type()));
            ASSERT('C' == f(bslmf::Switch4<2,A,B,C,D>::Type()));
            ASSERT('D' == f(bslmf::Switch4<3,A,B,C,D>::Type()));

            if (verbose) printf("Testing SELECTOR out of range\n");
            ASSERT('0' == f(bslmf::Switch4<4,A,B,C,D>::Type()));
            ASSERT('0' == f(bslmf::Switch4<10,A,B,C,D>::Type()));
            ASSERT('0' == f(bslmf::Switch4<-1,A,B,C,D>::Type()));
        }

        if (verbose) printf("\tTesting 'bslmf_Switch5.\n");
        {
            if (verbose) printf("Testing SELECTOR in range\n");
            ASSERT('A' == f(bslmf::Switch5<0,A,B,C,D,E>::Type()));
            ASSERT('B' == f(bslmf::Switch5<1,A,B,C,D,E>::Type()));
            ASSERT('C' == f(bslmf::Switch5<2,A,B,C,D,E>::Type()));
            ASSERT('D' == f(bslmf::Switch5<3,A,B,C,D,E>::Type()));
            ASSERT('E' == f(bslmf::Switch5<4,A,B,C,D,E>::Type()));

            if (verbose) printf("Testing SELECTOR out of range\n");
            ASSERT('0' == f(bslmf::Switch5<5,A,B,C,D,E>::Type()));
            ASSERT('0' == f(bslmf::Switch5<10,A,B,C,D,E>::Type()));
            ASSERT('0' == f(bslmf::Switch5<-1,A,B,C,D,E>::Type()));
        }

        if (verbose) printf("\tTesting 'bslmf_Switch6.\n");
        {
            if (verbose) printf("Testing SELECTOR in range\n");
            ASSERT('A' == f(bslmf::Switch6<0,A,B,C,D,E,F>::Type()));
            ASSERT('B' == f(bslmf::Switch6<1,A,B,C,D,E,F>::Type()));
            ASSERT('C' == f(bslmf::Switch6<2,A,B,C,D,E,F>::Type()));
            ASSERT('D' == f(bslmf::Switch6<3,A,B,C,D,E,F>::Type()));
            ASSERT('E' == f(bslmf::Switch6<4,A,B,C,D,E,F>::Type()));
            ASSERT('F' == f(bslmf::Switch6<5,A,B,C,D,E,F>::Type()));

            if (verbose) printf("Testing SELECTOR out of range\n");
            ASSERT('0' == f(bslmf::Switch6<6,A,B,C,D,E,F>::Type()));
            ASSERT('0' == f(bslmf::Switch6<10,A,B,C,D,E,F>::Type()));
            ASSERT('0' == f(bslmf::Switch6<-1,A,B,C,D,E,F>::Type()));
        }

        if (verbose) printf("\tTesting 'bslmf_Switch7.\n");
        {
            if (verbose) printf("Testing SELECTOR in range\n");
            ASSERT('A' == f(bslmf::Switch7<0,A,B,C,D,E,F,G>::Type()));
            ASSERT('B' == f(bslmf::Switch7<1,A,B,C,D,E,F,G>::Type()));
            ASSERT('C' == f(bslmf::Switch7<2,A,B,C,D,E,F,G>::Type()));
            ASSERT('D' == f(bslmf::Switch7<3,A,B,C,D,E,F,G>::Type()));
            ASSERT('E' == f(bslmf::Switch7<4,A,B,C,D,E,F,G>::Type()));
            ASSERT('F' == f(bslmf::Switch7<5,A,B,C,D,E,F,G>::Type()));
            ASSERT('G' == f(bslmf::Switch7<6,A,B,C,D,E,F,G>::Type()));

            if (verbose) printf("Testing SELECTOR out of range\n");
            ASSERT('0' == f(bslmf::Switch7<7,A,B,C,D,E,F,G>::Type()));
            ASSERT('0' == f(bslmf::Switch7<10,A,B,C,D,E,F,G>::Type()));
            ASSERT('0' == f(bslmf::Switch7<-1,A,B,C,D,E,F,G>::Type()));
        }

        if (verbose) printf("\tTesting 'bslmf_Switch8.\n");
        {
            if (verbose) printf("Testing SELECTOR in range\n");
            ASSERT('A' == f(bslmf::Switch8<0,A,B,C,D,E,F,G,H>::Type()));
            ASSERT('B' == f(bslmf::Switch8<1,A,B,C,D,E,F,G,H>::Type()));
            ASSERT('C' == f(bslmf::Switch8<2,A,B,C,D,E,F,G,H>::Type()));
            ASSERT('D' == f(bslmf::Switch8<3,A,B,C,D,E,F,G,H>::Type()));
            ASSERT('E' == f(bslmf::Switch8<4,A,B,C,D,E,F,G,H>::Type()));
            ASSERT('F' == f(bslmf::Switch8<5,A,B,C,D,E,F,G,H>::Type()));
            ASSERT('G' == f(bslmf::Switch8<6,A,B,C,D,E,F,G,H>::Type()));
            ASSERT('H' == f(bslmf::Switch8<7,A,B,C,D,E,F,G,H>::Type()));

            if (verbose) printf("Testing SELECTOR out of range\n");
            ASSERT('0' == f(bslmf::Switch8<8,A,B,C,D,E,F,G,H>::Type()));
            ASSERT('0' == f(bslmf::Switch8<10,A,B,C,D,E,F,G,H>::Type()));
            ASSERT('0' == f(bslmf::Switch8<-1,A,B,C,D,E,F,G,H>::Type()));
        }

        if (verbose) printf("\tTesting 'bslmf_Switch9.\n");
        {
            if (verbose) printf("Testing SELECTOR in range\n");
            ASSERT('A' == f(bslmf::Switch9<0,A,B,C,D,E,F,G,H,I>::Type()));
            ASSERT('B' == f(bslmf::Switch9<1,A,B,C,D,E,F,G,H,I>::Type()));
            ASSERT('C' == f(bslmf::Switch9<2,A,B,C,D,E,F,G,H,I>::Type()));
            ASSERT('D' == f(bslmf::Switch9<3,A,B,C,D,E,F,G,H,I>::Type()));
            ASSERT('E' == f(bslmf::Switch9<4,A,B,C,D,E,F,G,H,I>::Type()));
            ASSERT('F' == f(bslmf::Switch9<5,A,B,C,D,E,F,G,H,I>::Type()));
            ASSERT('G' == f(bslmf::Switch9<6,A,B,C,D,E,F,G,H,I>::Type()));
            ASSERT('H' == f(bslmf::Switch9<7,A,B,C,D,E,F,G,H,I>::Type()));
            ASSERT('I' == f(bslmf::Switch9<8,A,B,C,D,E,F,G,H,I>::Type()));

            if (verbose) printf("Testing SELECTOR out of range\n");
            ASSERT('0' == f(bslmf::Switch9<9,A,B,C,D,E,F,G,H,I>::Type()));
            ASSERT('0' == f(bslmf::Switch9<10,A,B,C,D,E,F,G,H,I>::Type()));
            ASSERT('0' == f(bslmf::Switch9<-1,A,B,C,D,E,F,G,H,I>::Type()));
        }

      } break;
      case 1: {
        // --------------------------------------------------------------------
        // FUNCTIONALITY TEST
        //
        // Concerns:
        //   Returns the right type for 'SELECTOR' values 0-4.
        //   Returns 'bslmf::Nil' for 'SELECTOR' < 0 or 4 < 'SELECTOR'
        //   Returns 'bslmf::Nil' if 'SELECTOR' > number of template arguments
        //
        // Plan:
        //   Define 10 overloaded versions of a function, 'f' returning
        //       a different letter for each of the types 'A' - 'J' and
        //       returning '0' for 'bslmf::Nil'.
        //   Instantiate with 10 different types each value in the range 0-9 as
        //       'SELECTOR'.  Call an 'f' function on the result to verify the
        //       type.  Instantiate with 'SELECTOR' out of range 0-9 and verify
        //       that 'f' returns '0'.
        //   Instantiate with fewer than 9 different types and verify results.
        //
        // Testing:
        //     bslmf::Switch<SELECTOR,T0,T1,T2,T3,T4,T5,T6,T7,T8,T9>::Type
        // --------------------------------------------------------------------

        if (verbose) printf("\nFUNCTIONALITY TEST"
                                 "\n==================\n");

        if (verbose) printf("Testing SELECTOR in range\n");
        ASSERT('A' == f(bslmf::Switch<0,A,B,C,D,E,F,G,H,I,J>::Type()));
        ASSERT('B' == f(bslmf::Switch<1,A,B,C,D,E,F,G,H,I,J>::Type()));
        ASSERT('C' == f(bslmf::Switch<2,A,B,C,D,E,F,G,H,I,J>::Type()));
        ASSERT('D' == f(bslmf::Switch<3,A,B,C,D,E,F,G,H,I,J>::Type()));
        ASSERT('E' == f(bslmf::Switch<4,A,B,C,D,E,F,G,H,I,J>::Type()));
        ASSERT('F' == f(bslmf::Switch<5,A,B,C,D,E,F,G,H,I,J>::Type()));
        ASSERT('G' == f(bslmf::Switch<6,A,B,C,D,E,F,G,H,I,J>::Type()));
        ASSERT('H' == f(bslmf::Switch<7,A,B,C,D,E,F,G,H,I,J>::Type()));
        ASSERT('I' == f(bslmf::Switch<8,A,B,C,D,E,F,G,H,I,J>::Type()));
        ASSERT('J' == f(bslmf::Switch<9,A,B,C,D,E,F,G,H,I,J>::Type()));

        if (verbose) printf("Testing SELECTOR out of range\n");
        ASSERT('0' == f(bslmf::Switch<10,A,B,C,D,E,F,G,H,I,J>::Type()));
        ASSERT('0' == f(bslmf::Switch<100,A,B,C,D,E,F,G,H,I,J>::Type()));
        ASSERT('0' == f(bslmf::Switch<-1,A,B,C,D,E,F,G,H,I,J>::Type()));

        if (verbose) printf("Testing Default arguments\n");
        ASSERT('A' == f(bslmf::Switch<0,A>::Type()));
        ASSERT('0' == f(bslmf::Switch<1,A>::Type()));
        ASSERT('0' == f(bslmf::Switch<2,A>::Type()));
        ASSERT('0' == f(bslmf::Switch<3,A>::Type()));
        ASSERT('0' == f(bslmf::Switch<4,A>::Type()));
        ASSERT('0' == f(bslmf::Switch<5,A>::Type()));
        ASSERT('0' == f(bslmf::Switch<6,A>::Type()));
        ASSERT('0' == f(bslmf::Switch<7,A>::Type()));
        ASSERT('0' == f(bslmf::Switch<8,A>::Type()));
        ASSERT('0' == f(bslmf::Switch<10,A>::Type()));
        ASSERT('0' == f(bslmf::Switch<-1,A>::Type()));
        ASSERT('B' == f(bslmf::Switch<1,A,B>::Type()));
        ASSERT('0' == f(bslmf::Switch<2,A,B>::Type()));

        if (verbose) printf("Testing Repeated arguments\n");
        ASSERT('A' == f(bslmf::Switch<0,A,B,A,B,A,C,C,C,C>::Type()));
        ASSERT('B' == f(bslmf::Switch<1,A,B,A,B,A,C,C,C,C>::Type()));
        ASSERT('A' == f(bslmf::Switch<2,A,B,A,B,A,C,C,C,C>::Type()));
        ASSERT('B' == f(bslmf::Switch<3,A,B,A,B,A,C,C,C,C>::Type()));
        ASSERT('A' == f(bslmf::Switch<4,A,B,A,B,A,C,C,C,C>::Type()));
        ASSERT('C' == f(bslmf::Switch<5,A,B,A,B,A,C,C,C,C>::Type()));
        ASSERT('C' == f(bslmf::Switch<6,A,B,A,B,A,C,C,C,C>::Type()));
        ASSERT('C' == f(bslmf::Switch<7,A,B,A,B,A,C,C,C,C>::Type()));
        ASSERT('C' == f(bslmf::Switch<8,A,B,A,B,A,C,C,C,C>::Type()));
        ASSERT('0' == f(bslmf::Switch<9,A,B,A,B,A,C,C,C,C>::Type()));

      } break;

      default: {
        fprintf(stderr, "WARNING: CASE `%d' NOT FOUND.\n", test);
        testStatus = -1;
      }
    }

    if (testStatus > 0) {
        fprintf(stderr, "Error, non-zero test status = %d.\n", testStatus);
    }

    return testStatus;
}

// ----------------------------------------------------------------------------
// Copyright (C) 2013 Bloomberg L.P.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// ----------------------------- END-OF-FILE ----------------------------------
