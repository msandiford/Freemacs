
#include "strprim.h"

class eqPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        const MintString& a1 = args[1].getValue();
        const MintString& a2 = args[2].getValue();
        if (a1 == a2) {
            interp.returnString(is_active, args[3].getValue());
        } else {
            interp.returnString(is_active, args[4].getValue());
        } // if
    } // operator()
}; // eqPrim

class nePrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        const MintString& a1 = args[1].getValue();
        const MintString& a2 = args[2].getValue();
        if (a1 != a2) {
            interp.returnString(is_active, args[3].getValue());
        } else {
            interp.returnString(is_active, args[4].getValue());
        } // if
    } // operator()
}; // nePrim

class ncPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        const MintString& a1 = args[1].getValue();
        interp.returnInteger(is_active, a1.size());
    } // operator()
}; // ncPrim

class aoPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        const MintString& a1 = args[1].getValue();
        const MintString& a2 = args[2].getValue();
        if (a1 <= a2) {
            interp.returnString(is_active, args[3].getValue());
        } else {
            interp.returnString(is_active, args[4].getValue());
        } // if
    } // operator()
}; // aoPrim

class saPrim : public MintPrim {
public:
    saPrim() : _comma(",") { }
private:
    const MintString _comma;
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        MintString s;
        if (args.size() > 2) {
            // Skip SELF (arg[0]) and end marker (arg[size-1])
            MintArgList::const_iterator first = args.begin();
            std::advance(first, 1);
            MintArgList::const_iterator last  = args.begin();
            std::advance(last, args.size() - 1);
            MintArgList newargs;
            // Note that this introduces sort instability as items are reversed here.
            // We use front_inserter as this is way cheaper if we are using an slist.
            // Fortunately this sort instability does not matter, as mint does
            // not distinguish between string instances.
            std::copy(first, last, std::front_inserter(newargs));
            newargs.sort();
            for (MintArgList::const_iterator i = newargs.begin();
                 i != newargs.end(); ++i) {
                s.append(i->getValue());
                s.append(_comma);
            } // for
        } // if
        interp.returnString(is_active, s);
    } // operator()
}; // saPrim

class siPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        MintString ret;
        const MintString& form = interp.getForm(args[1].getValue());
        const MintString& orig = args[2].getValue();
        for (MintString::const_iterator i = orig.begin(); i != orig.end(); ++i) {
            umintchar_t index = static_cast<umintchar_t>(*i);
            if (index >= form.size()) {
                ret.append(1, static_cast<mintchar_t>(index));
            } else {
                ret.append(1, form[index]);
            } // else
        } // for
        interp.returnString(is_active, ret);
    } // operator()
}; // siPrim

class nlPrim : public MintPrim {
    static const MintString NEW_LINE;
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        interp.returnString(is_active, NEW_LINE);
    } // operator()
}; // nlPrim

const MintString nlPrim::NEW_LINE = "\n";


void registerStrPrims(Mint& interp) {
    interp.addPrim("==", new eqPrim);
    interp.addPrim("!=", new nePrim);
    interp.addPrim("nc", new ncPrim);
    interp.addPrim("a?", new aoPrim);
    interp.addPrim("sa", new saPrim);
    interp.addPrim("si", new siPrim);
    interp.addPrim("nl", new nlPrim);
} // registerStrPrims

// EOF
