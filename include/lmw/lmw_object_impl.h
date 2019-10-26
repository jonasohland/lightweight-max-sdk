#pragma once

lmw::message::message(object_base* owner, const symbol& name,
                      const method& handler)
    : m_name(name), m_type("anything")
{
    lmw_internal_init(owner, handler);
}

lmw::message::message(object_base* owner, const symbol& name,
                      const symbol& type, const method& handler)
    : m_name(name), m_type(type)
{
    lmw_internal_init(owner, handler);
}

lmw::message::message(object_base* owner, const symbol& name,
                      const symbol& type, const std::string& description,
                      const method& handler)
    : m_name(name), m_type(type), m_description(description)
{
    lmw_internal_init(owner, handler);
}

inline void lmw::message::lmw_internal_init(object_base* owner, method m)
{
    owner->lmw_internal_assign(this);
    executor.set_handler(m);
}

inline void lmw::outlet::lmw_internal_create(object_base* obj, long index,
                                             std::size_t total)
{
    lmw_internal_set_owner(obj->native_handle());

    m_outlet = static_cast<c74::max::t_outlet*>(c74::max::outlet_new(
        owner(), any() ? nullptr : static_cast<const char*>(type())));
}

inline void lmw::inlet::lmw_internal_create(object_base* obj, long index,
                                            std::size_t total)
{
    lmw_internal_set_owner(obj->native_handle());

    if (!(type() == sym::signal) && !(type() == sym::multichannelsignal)) {

        if (index == 0) return;

        m_inlet_proxy = c74::max::proxy_new(owner(), total - index, nullptr);
    }
    else
        m_inlet = c74::max::inlet_new(owner(), type());
}