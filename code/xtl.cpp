//
//  Mach7: Pattern Matching Library for C++
//
//  Copyright 2011-2013, Texas A&M University.
//  Copyright 2014 Yuriy Solodkyy.
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//
//      * Redistributions of source code must retain the above copyright
//        notice, this list of conditions and the following disclaimer.
//
//      * Redistributions in binary form must reproduce the above copyright
//        notice, this list of conditions and the following disclaimer in the
//        documentation and/or other materials provided with the distribution.
//
//      * Neither the names of Mach7 project nor the names of its contributors
//        may be used to endorse or promote products derived from this software
//        without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
//  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
//  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY
//  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

///
/// \file
///
/// This file is a part of Mach7 library test suite.
///
/// \author Yuriy Solodkyy <yuriy.solodkyy@gmail.com>
///
/// \see https://parasol.tamu.edu/xtl/
/// \see https://parasol.tamu.edu/mach7/
/// \see https://github.com/solodon4/Mach7
/// \see https://github.com/solodon4/SELL
///

#include <iostream>
#include "boost/variant.hpp"
#include "xtl.hpp"

namespace xtl
{
    template <class S, class T, class... Ts>
    struct is_subtype<S,boost::variant<T,Ts...>>
    {
        static const bool value = is_subtype<S,T>::value
                               || is_subtype<S,boost::variant<Ts...>>::value;
    };

    template <class S>
    struct is_subtype<S,boost::variant<>> : std::false_type {};

    template <class... Ts, class S>
    boost::variant<Ts...> subtype_cast_impl(target<boost::variant<Ts...>> v, const S& s)
    {
        return boost::variant<Ts...>(s); // FIX: Actually this should be boost::variant<Ts...>(xtl::subtype_cast<Ti>(s)) where S <: Ti
    }

    template <typename T>
    struct is_subtype_visitor : public boost::static_visitor<T*>
    {
        template <typename S>
        T* operator()(S& s) const
        {
            return xtl::subtype_dynamic_cast<T*>(&s);
        }
    };

    template <class T, class... Ts>
    typename std::enable_if<xtl::is_subtype<T, boost::variant<Ts...>>::value, T*>::type
    subtype_dynamic_cast_impl(target<T*>, boost::variant<Ts...>* pv)
    {
        is_subtype_visitor<T> visitor;
        return boost::apply_visitor(visitor, *pv);
    }
}

static_assert(xtl::is_subtype<int,int>::value,"No reflexivity");

struct A             { int    a; virtual void foo() { std::cout << "A" << std::endl; } };
struct B : A         { int    b; virtual void foo() { std::cout << "B" << std::endl; } };
struct C : A         { double c; virtual void foo() { std::cout << "C" << std::endl; } };
struct D : C, B      { float  d; virtual void foo() { std::cout << "D" << std::endl; } };
struct X : virtual A { char   x; virtual void foo() { std::cout << "X" << std::endl; } };
struct Y : virtual A { short  y; virtual void foo() { std::cout << "Y" << std::endl; } };
struct Z : X, Y      { char   z; virtual void foo() { std::cout << "Z" << std::endl; } };

int main()
{
    A a;
    B b;
    a = xtl::subtype_cast<A>(b);
    //b = xtl::subtype_cast<B>(a); // error

    A* qa = xtl::subtype_cast<A*>(&b);
    //B* qb = xtl::subtype_cast<B*>(&a); // error
    B* qb = xtl::subtype_dynamic_cast<B*>(qa);
    qa->foo();
    if (qb) qb->foo();

    A* pA = new A;
    B* pB = new B;
    C* pC = new C;
    D* pD = new D;
    X* pX = new X;
    Y* pY = new Y;
    Z* pZ = new Z;

    A* pa = xtl::subtype_cast<A*>(pA);
    A* pb = xtl::subtype_cast<A*>(pB);
    A* pc = xtl::subtype_cast<A*>(pC);
    A* pdb= xtl::subtype_cast<A*>((B*)pD);
    A* pdc= xtl::subtype_cast<A*>((C*)pD);
    A* px = xtl::subtype_cast<A*>(pX);
    A* py = xtl::subtype_cast<A*>(pY);
    A* pz = xtl::subtype_cast<A*>(pZ);

    if (A* p = xtl::subtype_dynamic_cast<A*>(pa)) p->foo();
    if (B* p = xtl::subtype_dynamic_cast<B*>(pb)) p->foo();
    if (C* p = xtl::subtype_dynamic_cast<C*>(pc)) p->foo();
    if (D* p = xtl::subtype_dynamic_cast<D*>(pdb))p->foo();
    if (D* p = xtl::subtype_dynamic_cast<D*>(pdc))p->foo();
    if (X* p = xtl::subtype_dynamic_cast<X*>(px)) p->foo();
    if (Y* p = xtl::subtype_dynamic_cast<Y*>(py)) p->foo();
    if (Z* p = xtl::subtype_dynamic_cast<Z*>(pz)) p->foo();
    if (B* p = xtl::subtype_dynamic_cast<B*>(pdc))p->foo();
    if (C* p = xtl::subtype_dynamic_cast<C*>(pdb))p->foo();

    static_assert(xtl::is_subtype<B,A>::value,"Subclassing");

    static_assert(xtl::is_subtype<int,boost::variant<double,float,int,unsigned int*>>::value, "Not a subtype");

    typedef boost::variant<double,float,int,unsigned int*> my_variant_1;
    my_variant_1 v1 = xtl::subtype_cast<my_variant_1>(3.1415);
    std::cout << '(' << v1.which() << ',' << v1 << ')' << std::endl;

    my_variant_1 v2 = xtl::subtype_cast<my_variant_1>(42);
    std::cout << '(' << v2.which() << ',' << v2 << ')' << std::endl;

    int* p1 = xtl::subtype_dynamic_cast<int*>(&v1);
    std::cout << '(' << p1 << ',' << (p1 ? *p1 : 99999) << ')' << std::endl;

    int* p2 = xtl::subtype_dynamic_cast<int*>(&v2);
    std::cout << '(' << p2 << ',' << (p2 ? *p2 : 99999) << ')' << std::endl;
}