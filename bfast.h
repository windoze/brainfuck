//
//  bfast.h
//  brainfuck
//
//  Created by Xu Chen on 12-9-8.
//  Copyright (c) 2012 Xu Chen. All rights reserved.
//

#include <string>
#include <vector>
#include <ostream>
#include <boost/shared_ptr.hpp>
#include <boost/variant.hpp>

#ifndef brainfuck_bfast_h
#define brainfuck_bfast_h

namespace brainfuck {
    namespace ast {
        struct MoveLeft {
            // Minimized container interface, used by Boost.Spirit
            typedef size_t value_type;
            inline int end() const { return 0; }
            inline void insert(int /*unused*/, char /*unused*/) { count_++; }

            inline MoveLeft() : count_(0) {}
            size_t count_;
        };
        
        inline std::ostream &operator << (std::ostream &os, const MoveLeft &n) {
        os << "MoveLeft[" << n.count_ << ']';
        return os;
        }
        
        struct MoveRight {
            // Minimized container interface, used by Boost.Spirit
            typedef size_t value_type;
            inline int end() const { return 0; }
            inline void insert(int /*unused*/, char /*unused*/) { count_++; }
            
            inline MoveRight() : count_(0) {}
            size_t count_;
        };
        
        inline std::ostream &operator<<(std::ostream &os, const MoveRight &n) {
            os << "MoveRight[" << n.count_ << ']';
            return os;
        }
        
        struct Add {
            // Minimized container interface, used by Boost.Spirit
            typedef size_t value_type;
            inline int end() const { return 0; }
            inline void insert(int /*unused*/, char c) { if(c=='+') count_++; }
            
            inline Add() : count_(0) {}
            size_t count_;
        };
        
        inline std::ostream &operator<<(std::ostream &os, const Add &n) {
            os << "Add[" << n.count_ << ']';
            return os;
        }
        
        struct Minus {
            // Minimized container interface, used by Boost.Spirit
            typedef size_t value_type;
            inline int end() const { return 0; }
            inline void insert(int /*unused*/, char c) { if(c=='-') count_++; }
            
            inline Minus() : count_(0) {}
            size_t count_;
        };
        
        inline std::ostream &operator<<(std::ostream &os, const Minus &n) {
            os << "Minus[" << n.count_ << ']';
            return os;
        }
        
        struct Input {
            inline Input() {}
            inline Input(char /* unused */) {}
        };
        
        inline std::ostream &operator<<(std::ostream &os, const Input &n) {
            os << "Input";
            return os;
        }
        
        struct Output {
            inline Output() {}
            inline Output(char /* unused */) {}
        };
        
        inline std::ostream &operator<<(std::ostream &os, const Output &n) {
            os << "Output";
            return os;
        }
        
        typedef boost::variant<MoveLeft, MoveRight, Add, Minus, Input, Output> Primitive;
        
        struct Loop;
        typedef boost::variant<Primitive, Loop> Command;
        typedef std::vector<Command> Commands;
        typedef boost::shared_ptr<Commands> Commands_ptr;
        
        struct Loop {
            // Minimized container interface, used by Boost.Spirit
            typedef std::vector<Command> value_type;
            void insert(Commands::iterator i, const Commands &cmds);
            Commands::iterator end();
            Commands::const_iterator end() const;
            
            Loop();
            Commands_ptr commands_;
        };
        
        inline Loop::Loop() : commands_(new Commands)
        {}
        
        inline void Loop::insert(Commands::iterator i, const Commands &cmds) {
            commands_->insert(i, cmds.begin(), cmds.end());
        }
        
        inline Commands::iterator Loop::end() {
            return commands_->end();
        }
        
        inline Commands::const_iterator Loop::end() const {
            return commands_->end();
        }
        
        inline std::ostream &operator<<(std::ostream &os, const Loop &n) {
            os << "Branch[";
            for(Commands::const_iterator i=n.commands_->begin(); i!=n.commands_->end(); ++i){
                if(i!=n.commands_->begin()) os << ',';
                os << *i;
            }
            os << ']';
            return os;
        }
        
        typedef Commands Program;
        
        inline std::ostream &operator<<(std::ostream &os, const Program &n) {
            os << "Program[";
            for(Program::const_iterator i=n.begin(); i!=n.end(); ++i){
                if(i!=n.begin()) os << ',';
                os << *i;
            }
            os << ']';
            return os;
        }
    }   // End of namespace ast
}   // End of namespace brainfuck

#endif
