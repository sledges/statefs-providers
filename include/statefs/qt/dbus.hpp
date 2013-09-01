#ifndef _STATEFS_QT_DBUS_HPP_
#define _STATEFS_QT_DBUS_HPP_

#include <QDBusPendingReply>
#include <QDBusServiceWatcher>
#include <stdexcept>
#include <tuple>
#include <memory>

namespace statefs { namespace qt {

template <typename T>
QDBusPendingReply<T> sync(QDBusPendingReply<T> &&reply)
{
    QDBusPendingCallWatcher watcher(reply);
    watcher.waitForFinished();
    if (!watcher.isFinished())
        throw std::logic_error("D-Bus request is not executed");
    return reply;
}

template <size_t Pos, typename T>
struct TupleDBus
{
    static void output(QDBusArgument &argument, T const &src)
    {
        argument << std::get<std::tuple_size<T>::value - Pos>(src);
        TupleDBus<Pos - 1, T>::output(argument, src);
    }

    static void input(QDBusArgument const &argument, T &dst)
    {
        argument >> std::get<std::tuple_size<T>::value - Pos>(dst);
        TupleDBus<Pos - 1, T>::input(argument, dst);
    }
};

template <typename T>
struct TupleDBus<0, T>
{
    static void output(QDBusArgument &, T const &)
    {
        // do nothing, after the last element
    }

    static void input(QDBusArgument const &, T &)
    {
        // do nothing, after the last element
    }
};


class ServiceWatch : public QObject
{
    Q_OBJECT;
public:
    ServiceWatch(QDBusConnection &bus, QString const &service)
        : bus_(bus), service_(service)
    {}

    virtual ~ServiceWatch() {}

    template <typename RegT, typename UnregT>
    void init(RegT onRegister, UnregT onUnregister)
    {
        if (watcher_)
            return;

        watcher_.reset(new QDBusServiceWatcher(service_, bus_));
        // , QDBusServiceWatcher::WatchForRegistration));

        auto cb = [onRegister, onUnregister]
            (const QString & serviceName
             , const QString & oldOwner
             , const QString & newOwner) {
            if (newOwner == "") {
                qDebug() << serviceName << " is unregistered";
                onUnregister();
            } else {
                qDebug() << serviceName << " is registered";
                onRegister();
            }
        };
        connect(watcher_.get(), &QDBusServiceWatcher::serviceOwnerChanged
                , cb);
    }

private:
    QDBusConnection &bus_;
    QString service_;
    std::unique_ptr<QDBusServiceWatcher> watcher_;
};

}}

template <typename ... Args>
QDBusArgument & operator << (QDBusArgument &argument, std::tuple<Args...> const &src)
{
    using statefs::qt::TupleDBus;
    typedef std::tuple<Args...> tuple_type;
    argument.beginStructure();
    TupleDBus<std::tuple_size<tuple_type>::value
              , tuple_type>::output(argument, src);
    argument.endStructure();
    return argument;
}

template <typename ... Args>
QDBusArgument const& operator >> (QDBusArgument const &argument, std::tuple<Args...> &dst)
{
    using statefs::qt::TupleDBus;
    typedef std::tuple<Args...> tuple_type;
    argument.beginStructure();
    TupleDBus<std::tuple_size<tuple_type>::value
              , tuple_type>::input(argument, dst);
    argument.endStructure();
    return argument;
}

#endif // _STATEFS_QT_DBUS_HPP_
