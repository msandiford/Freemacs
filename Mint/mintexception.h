/*
 * Copyright 2020 Martin Sandiford
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to: Free Software Foundation
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _MINTEXCEPTION_H
#define _MINTEXCEPTION_H

#include <exception>

class MintException : public std::exception {
public:
    explicit MintException(const std::string& err) : _what(err) {}
    virtual ~MintException() throw() { }
    virtual const char* what() const throw() { return _what.c_str(); }
private:
    std::string _what;
}; // MintException

#endif // MINTEXCEPTION_H

// EOF
