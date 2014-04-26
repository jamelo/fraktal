#ifndef Wrapper_H
#define Wrapper_H

#include <QObject>

template<class T>
class Wrapper : public QObject
{
private:
    T m_object;

public:
    Wrapper(QObject* parent, T& object) :
        QObject(parent),
        m_object(object)
    { }

    const T& get() const    { return m_object; }
    T& get()                { return m_object; }
};

#endif
