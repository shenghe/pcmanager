///////////////////////////////////////////////////////////////////////////////
//
//  FileName    :   pattern.h 
//  Version     :   1.0
//  Author      :   Liu Shuo
//  Date        :   2008.12.25
//  Comment     :   pattern match by suffix based approach
//
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include <iostream>
#include <vector>
#include <map>

class   pattern
{
#define INIT_MEMBER()               \
    m_trie( NULL ),                 \
    m_terminal( NULL ),             \
    m_lmin( 0 ),                    \
    m_lmax( 0 ),                    \
    m_tsize( 0 ),                   \
    m_enumMethod( enum_invalid ),   \
    m_bprocess( false )
#define ALPHA_SET_SIZE              256
#define WILDCARD                    '?'

public:
    typedef unsigned char               char_type;
    typedef size_t                      size_type;
    typedef std::vector<char_type>      pattern_type, terminal_type;
    typedef std::vector<pattern_type>   pattern_array_type;
    typedef std::multimap<size_type, size_type> terminal_state_type;
    enum method_type
    {
        enum_invalid,
        enum_horspool,
        enum_sethorspool,
        enum_wumanber
    };
public:
    pattern() : INIT_MEMBER()
    {
        _innerinit();
    }

    pattern( const pattern &right )
    {
        copy( right );
    }

    pattern &operator =( const pattern &right )
    {
        if ( this == &right )
        {
            return *this;
        }

        if ( m_trie )
        {
            delete[] m_trie;
        }

        if ( m_terminal )
        {
            delete[] m_terminal;
        }

        copy ( right );
        return *this;
    }

    ~pattern()
    {
        if ( m_trie )
        {
            delete[] m_trie;
        }

        if ( m_terminal )
        {
            delete[] m_terminal;
        }
    }
public:
    // return number of patterns
    int add_pattern( const pattern_type &pattern )
    {
        int ret = -1;

        if ( pattern.size() )
        {
            m_vpattern.push_back( pattern );

            if ( 1 == m_vpattern.size() )
            {
                m_lmax = ( m_lmin = pattern.size() );
                m_tsize = m_lmax;
                ret = 1;
            }
            else
            {
                m_tsize += pattern.size();
                
                if ( pattern.size() < m_lmin )
                {
                    m_lmin = pattern.size();
                }

                if ( pattern.size() > m_lmax )
                {
                    m_lmax = pattern.size();
                }

                ret = ( int )m_vpattern.size();
            }
        }

        if ( -1 != ret )
        {
            update_method_type ();
        }
        return ret;
    }

    int add_pattern( char_type *arr_pattern, size_type esize )
    {
        pattern_type    tmp;
        if ( ( NULL == arr_pattern ) || ( 0 == esize ) )
        {
            return -1;
        }

        tmp.resize( esize );
        memcpy( &tmp[ 0 ], arr_pattern, sizeof( char_type ) * esize );

        return add_pattern ( tmp );
    }

    int add_pattern( const pattern_array_type &arr_pattern )
    {
        size_type number = arr_pattern.size();
        for ( size_type i = 0; i < number; i++ )
        {
            if ( -1 == add_pattern ( arr_pattern[ i ] ) )
            {
                return int( i );
            }
        }

        return int( number );
    }

    void preprocess()
    {
        badcharshift ();
        buildtrie ();
        m_bprocess = true;
    }

    int match( 
        const char_type *buff, 
        const size_type length, 
        std::vector<size_type> &set
        )
    {
        int    nret = -1;

        if ( ( NULL == buff ) || ( 0 == length ) || !m_bprocess )
        {
            return nret;
        }

        switch( m_enumMethod )
        {
        case    enum_horspool:
            {
                nret = horspool ( buff, length );
                if ( -1 != nret )
                {
                    set.push_back ( 0 );
                }
                break;
            }
        case    enum_sethorspool:
            {
                nret = sethorspool ( buff, length, set );
                break;
            }
        case    enum_wumanber:
        default:
            {
                break;
            }
        }

        return nret;
    }

	int match_all( 
		const char_type *buff, 
		const size_type length, 
		std::vector<size_type> &set
		)
	{
		int    nret = -1;

		if ( ( NULL == buff ) || ( 0 == length ) || !m_bprocess )
		{
			return nret;
		}

		switch( m_enumMethod )
		{
		case    enum_horspool:
			{
				nret = horspool ( buff, length );
				if ( -1 != nret )
				{
					set.push_back ( 0 );
				}
				break;
			}
		case    enum_sethorspool:
			{
				nret = sethorspool_all ( buff, length, set );
				break;
			}
		case    enum_wumanber:
		default:
			{
				break;
			}
		}

		return nret;
	}

private:
    void _innerinit()
    {
        for ( int i = 0; i < ALPHA_SET_SIZE; i++ )
        {
            m_alphaset[ i ] = char_type( i );
            m_bcshift[ i ] = size_type( -1 );
        }
    }

    int imemcmp( const unsigned char* pattern, const unsigned char* buf, size_type count )
    {
        int nRet = 0;
        for ( size_type i = 0; i < count; i++ )
        {
            if ( ( '?' == pattern[ i ] ) || ( pattern[ i ] == buf[ i ] ) )
            {
                continue;
            }

            nRet =  int( pattern[ i ] ) - int( buf[ i ] );
            break;
        }

        return nRet;
    }

    void copy( const pattern &right )
    {
        m_vpattern = right.m_vpattern;
        m_lmin = right.m_lmin;
        m_lmax = right.m_lmax;
        m_tsize = right.m_tsize;
        m_enumMethod = right.m_enumMethod;
        m_bprocess = right.m_bprocess;

        memcpy( m_alphaset, right.m_alphaset, sizeof( char_type ) * ALPHA_SET_SIZE );

        if( m_bprocess )
        {
            memcpy( m_bcshift, right.m_bcshift, sizeof( size_type ) * ALPHA_SET_SIZE );

            m_trie = new size_type[ m_tsize + 1 ][ ALPHA_SET_SIZE ];
            m_terminal = new size_type[ m_tsize + 1 ];

            if ( m_trie && m_terminal )
            {
                memcpy( m_trie, right.m_trie, sizeof( size_type ) * ALPHA_SET_SIZE * ( m_tsize + 1 ) );
                memcpy( m_terminal, right.m_terminal, sizeof( size_type ) * ( m_tsize + 1 ) );
                m_stateset = right.m_stateset;
            }
        }
        
    }

    // get the top bound of logarithm only for the case while number not less than base
    bool  top_log( size_type base, size_type number, size_type *top )
    {
        size_type   log = 0;
        size_type   tmp;

        if ( NULL == top )
        {
            return false;
        }

        *top = 0;

        if ( 0 == base )
        {
            return false;
        }

        if ( number < base )
        {
            return false;
        }

        tmp = number;
        while ( tmp >= base )
        {
            log++;
            tmp /= base;
        }
        *top = log;
        return true;
    }

    size_type  compute_wmblock_size()
    {
        size_type  gene = 0;
        size_type  blocksize = 0;

        // this is a preferable data, mustn't modify.
        gene = 2 * m_lmin * m_vpattern.size();

        if ( gene < ALPHA_SET_SIZE )
        {
            return 1;
        }

        if ( !top_log ( ALPHA_SET_SIZE, gene, &blocksize ) )
        {
            return 1;
        }

        return blocksize;
    }

    void update_method_type()
    {
        if ( 0 == m_vpattern.size() )
        {
            m_enumMethod = enum_invalid;
        }
        else if ( 1 == m_vpattern.size() )
        {
            m_enumMethod = enum_horspool;
        }
        else
        {
            if ( 1 == compute_wmblock_size () )
            {
                m_enumMethod = enum_sethorspool;
            }
            else
            {
                m_enumMethod = enum_wumanber;
            }
        }
    }

    void badcharshift()
    {
        size_type   number = m_vpattern.size();
        
        for ( size_type i = 0; i < ALPHA_SET_SIZE; i++ )
        {
            m_bcshift[ m_alphaset[ i ] ] = m_lmin;
        }

        for ( size_type i = 0; i < number; i++ )
        {
            pattern_type    pattern = m_vpattern[ i ];
            size_type       length = pattern.size();

            // process from last to begin, when wildcard occur every bcshfit must be set
            for ( size_type j = ( length - 1 ); j > 0 ; j-- )
            {
                if ( pattern[ j - 1 ] != WILDCARD )
                {
                    m_bcshift[ pattern[ j - 1 ] ] = min( m_bcshift[ pattern[ j - 1 ] ], length - j );
                }
                else
                {
                    for ( size_type k = 0; k < ALPHA_SET_SIZE; k++ )
                    {
                        m_bcshift[ m_alphaset[ k ] ] = min( m_bcshift[ m_alphaset[ k ] ], length - j );
                    }
                    // we don't need process other character, because every bcshift has been set
                    break;
                }
            }
        }
        
    }

    void buildtrie()
    {
        size_type   number = m_vpattern.size();
        size_type   r = 0;
        size_type   state = 0;

        m_trie = new size_type[ m_tsize + 1 ][ ALPHA_SET_SIZE ];
        m_terminal = new size_type[ m_tsize + 1 ];
        memset( m_trie, -1, sizeof( size_type ) * ALPHA_SET_SIZE * ( m_tsize + 1 ) );
        memset( m_terminal, 0, sizeof( size_type ) * ( m_tsize + 1 ) );

        for ( r = 0; r < number; r++ )
        {
            size_type   cur = 0;
            pattern_type    pattern = m_vpattern[ r ];
            size_type   length = pattern.size();
            size_type   i = length;

            while ( ( i > 0 ) && ( size_type( -1 ) != m_trie[ cur ][ pattern[ i - 1 ] ] ) )
            {
                cur = m_trie[ cur ][ pattern[ i - 1 ] ];
                i--;
            }

            while ( i > 0 )
            {
                state++;

                if ( WILDCARD == pattern[  i - 1  ] )
                {
                    for ( size_type j = 0; j < ALPHA_SET_SIZE; j++ )
                    {
                        if ( size_type( -1 ) == m_trie[ cur ][ m_alphaset[ j ] ] )
                        {
                            m_trie[ cur ][ m_alphaset[ j ] ] = state;
                        }
                    }
                }
                else
                {
                    m_trie[ cur ][ pattern[  i - 1  ] ] = state;
                }
                cur = state;
                i--;
            }

            m_terminal[ cur ] = 1;
            m_stateset.insert( std::make_pair ( cur, r ) );
        }
        
        
    }

    int horspool( const char_type *buff, const size_type length )
    {
        char_type   c;
        size_type   m, pos; // m  = pattern.size()
        char_type   *pattern = NULL;

        if ( m_vpattern.size() != 1 )
        {
            return -1;
        }

        if( 0 == m_vpattern[ 0 ].size() )
        {
            return -1;
        }

        pos = 0;
        m = m_vpattern[ 0 ].size();
        pattern = &m_vpattern[ 0 ][ 0 ];

        while ( ( length >= m ) && ( pos <= ( length - m ) ) )
        {
            c = buff[ pos + m - 1 ];
            if ( c == pattern[ m - 1 ] 
                && ( 0 == imemcmp( pattern, &buff[ pos ], m - 1 ) ) )
            {
                return int( size_type( buff ) + pos );
            }

            pos += m_bcshift[ c ];
        }
        return -1;
    }

    int sethorspool( 
        const char_type *buff, 
        const size_type length, 
        std::vector<size_type> &set
        )
    {
        size_type   pos;
        std::pair<terminal_state_type::iterator, terminal_state_type::iterator> range;
        size_type   *pterminal = &m_terminal[ 0 ];

        if ( ( m_vpattern.size() <= 1 ) 
            || ( 0 == m_lmin ) 
            || ( NULL == m_trie ) 
            || ( NULL == m_terminal ) )
        {
            return -1;
        }

        pos = m_lmin - 1;

        while ( pos < length )
        {
            size_type   i = 0;
            size_type   cur = 0;

            while ( ( pos >= i ) 
                && ( size_type( -1 ) != m_trie[ cur ][ buff[ pos - i ] ] ) )
            {
                char_type c = buff[ pos - i ];
                if( pterminal[ m_trie[ cur ][ c ] ] )
                {
                    range = m_stateset.equal_range( m_trie[ cur ][ c ] );
                    for ( terminal_state_type::iterator iter = range.first; iter != range.second; ++iter )
                    {
                        set.push_back ( iter->second );
                    }

                    return int( size_type( buff ) + pos - i );
                }

                cur = m_trie[ cur ][ c ];
                i++;
            }

            pos += m_bcshift[ buff[ pos ] ];
        }

        return -1;
    }

	int sethorspool_all( 
		const char_type *buff, 
		const size_type length, 
		std::vector<size_type> &set
		)
	{
		size_type   pos;
		std::pair<terminal_state_type::iterator, terminal_state_type::iterator> range;
		size_type   *pterminal = &m_terminal[ 0 ];

		if ( ( m_vpattern.size() <= 1 ) 
			|| ( 0 == m_lmin ) 
			|| ( NULL == m_trie ) 
			|| ( NULL == m_terminal ) )
		{
			return -1;
		}

		pos = m_lmin - 1;

		while ( pos < length )
		{
			size_type   i = 0;
			size_type   cur = 0;

			while ( ( pos >= i ) 
				&& ( size_type( -1 ) != m_trie[ cur ][ buff[ pos - i ] ] ) )
			{
				char_type c = buff[ pos - i ];
				if( pterminal[ m_trie[ cur ][ c ] ] )
				{
					range = m_stateset.equal_range( m_trie[ cur ][ c ] );
					for ( terminal_state_type::iterator iter = range.first; iter != range.second; ++iter )
					{
						set.push_back ( iter->second );
					}

					//return int( size_type( buff ) + pos - i );
				}

				cur = m_trie[ cur ][ c ];
				i++;
			}

			pos += m_bcshift[ buff[ pos ] ];
		}

		if(set.size() > 0)
			return set.size();
		return -1;
	}

private:
    char_type           m_alphaset[ ALPHA_SET_SIZE ];
    pattern_array_type  m_vpattern;
    size_type           (*m_trie)[ ALPHA_SET_SIZE ];
    size_type           *m_terminal;
    terminal_state_type m_stateset;
    size_type           m_bcshift[ ALPHA_SET_SIZE ];
    size_type           m_lmin;
    size_type           m_lmax;
    size_type           m_tsize;
    int                 m_enumMethod;
    bool                m_bprocess;
};