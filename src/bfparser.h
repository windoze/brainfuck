//  bfparser.h
//  brainfuck
//
//  Created by Xu Chen on 12-9-8.
//  Copyright (c) 2012 Xu Chen. All rights reserved.
//

//#define BOOST_SPIRIT_DEBUG
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/support_istream_iterator.hpp>
#include "bfast.h"

#ifndef brainfuck_bfparser_h
#define brainfuck_bfparser_h

namespace brainfuck {
    namespace parser {
        template<typename Iterator>
        struct skipper : boost::spirit::qi::grammar<Iterator> {
            skipper() : skipper::base_type(start) {
                using boost::spirit::ascii::char_;
                using boost::spirit::no_skip;
                using boost::spirit::lexeme;
                // All chars other than these 8 are ignored
                start = char_ - char_("-<>+,.[]")
                ;
            }
            boost::spirit::qi::rule<Iterator> start;
        };
        
        template<typename Iterator>
        struct parser : boost::spirit::qi::grammar<Iterator, ast::Program(), skipper<Iterator> > {
            parser() : parser::base_type(program) {
                using boost::spirit::ascii::char_;
                
                program = +command >> boost::spirit::eoi
                ;
                
                loop = '[' >> *command >> ']'
                ;
                
                command = loop
                        | primitive
                ;
                
                primitive = moveleft
                          | moveright
                          | add
                          | minus
                          | input
                          | output
                ;
                
                moveleft = +char_('<')
                ;
                
                moveright = +char_('>')
                ;
                
                add = +char_('+')
                ;
                
                minus = +char_('-')
                ;
                
                input = char_(',')
                ;
                
                output = char_('.')
                ;
                
                BOOST_SPIRIT_DEBUG_NODE(program);
                BOOST_SPIRIT_DEBUG_NODE(command);
                BOOST_SPIRIT_DEBUG_NODE(loop);
                BOOST_SPIRIT_DEBUG_NODE(primitive);
                BOOST_SPIRIT_DEBUG_NODE(moveleft);
                BOOST_SPIRIT_DEBUG_NODE(moveright);
                BOOST_SPIRIT_DEBUG_NODE(add);
                BOOST_SPIRIT_DEBUG_NODE(minus);
                BOOST_SPIRIT_DEBUG_NODE(input);
                BOOST_SPIRIT_DEBUG_NODE(output);
            }
            
            bool parse(Iterator first, Iterator last, ast::Program& prog) {
                return boost::spirit::qi::phrase_parse(first, last, *this, skipper<Iterator>(), prog);
            }
            
            boost::spirit::qi::rule<Iterator, ast::Program(), skipper<Iterator> > program;
            boost::spirit::qi::rule<Iterator, ast::Command(), skipper<Iterator> > command;
            boost::spirit::qi::rule<Iterator, ast::Loop(), skipper<Iterator> > loop;
            boost::spirit::qi::rule<Iterator, ast::Primitive(), skipper<Iterator> > primitive;
            boost::spirit::qi::rule<Iterator, ast::MoveLeft(), skipper<Iterator> > moveleft;
            boost::spirit::qi::rule<Iterator, ast::MoveRight(), skipper<Iterator> > moveright;
            boost::spirit::qi::rule<Iterator, ast::Add(), skipper<Iterator> > add;
            boost::spirit::qi::rule<Iterator, ast::Minus(), skipper<Iterator> > minus;
            boost::spirit::qi::rule<Iterator, ast::Input(), skipper<Iterator> > input;
            boost::spirit::qi::rule<Iterator, ast::Output(), skipper<Iterator> > output;
        };  // End of parser
        
        template<typename Iterator>
        bool parse(Iterator first, Iterator last, ast::Program &prog) {
            parser<Iterator> parser;
            return parser.parse(first, last, prog);
        }
    }   // End of namespace parser
}   // End of namespace brainfuck

#endif
