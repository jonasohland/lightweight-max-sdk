#pragma once

namespace lmw {

    class max_class_base {

      public:
        friend class message;
        friend class inlet;
        friend class outlet;

        template <typename user_class>
        friend void* wrapper_object_new(c74::max::t_class*, c74::max::t_symbol*,
                                        long, c74::max::t_atom*);

        template <typename user_class>
        friend void wrapper_dsp64_setup(c74::max::t_object* x,
                                        c74::max::t_object* dspman,
                                        short* count, double srate, long vsize,
                                        long flags, c74::max::t_perfroutine64);

        template <typename user_class>
        friend void wrapper_inputchanged_impl(c74::max::t_object*, long, long);

        template <typename user_class>
        friend long wrapper_multichanneloutputs_impl(c74::max::t_object*, long);

        virtual ~max_class_base()
        {
        }

        std::unordered_map<std::string, message*>& messages()
        {
            return m_messages;
        }

        bool has_method(const char* name) const noexcept
        {
            return m_messages.find(name) != m_messages.end();
        }
        
        const char* description_for_inlet(long inlet_idx)
        {
            return get_port_description(m_inlets, inlet_idx);
        }
        
        const char* description_for_outlet(long outlet_idx)
        {
            return get_port_description(m_outlets, outlet_idx);
        }
        
        bool inlet_is_hot(long inlet_idx)
        {
            if(LMW_UNLIKELY(inlet_idx >= m_inlets.size()))
                return false;
            
            return m_inlets[inlet_idx]->hot();
        }
        
        bool mc() const
        {
            for (const auto& inlet : m_inlets){
                if(inlet->mc())
                    return true;
            }
            
            for (const auto& outlet : m_outlets){
                if(outlet->mc())
                    return true;
            }
            
            return false;
        }

        std::size_t streams() const
        {
            std::size_t acc = 0;

            for (const auto& inlet : m_inlets)
                acc += (inlet->type() == sym::signal ||
                        inlet->type() == sym::multichannelsignal);
            
            return acc;
        }

        atom::vector call(const char* name, std::shared_ptr<atom::vector>&& args)
        {
            long inlet = c74::max::proxy_getinlet(native_handle());
            
            if (auto func = m_messages.find(name);
                LMW_LIKELY(func != m_messages.end()))
                func->second->call(
                    std::forward<std::shared_ptr<atom::vector>>(args), inlet);

            return {};
        }

        inline c74::max::t_object* native_handle() const noexcept
        {
            assert(static_cast<bool>(t_obj_instance_ptr));

            return t_obj_instance_ptr;
        }

        template <typename... Args>
        outlet_ptr make_outlet(Args&&... args)
        {
            return make_port<outlets>(
                sym::anything, std::forward<Args>(args)...);
        }

        template <typename... Args>
        inlet_ptr make_inlet(Args&&... args)
        {
            return make_port<outlets>(
                sym::anything, std::forward<Args>(args)...);
        }

        template <typename... Args>
        inlet_ptr make_typed_inlet(symbol ty, Args&&... args)
        {
            return make_port<inlet>(ty, std::forward<Args>(args)...);
        }
        
        template <typename... Args>
        outlet_ptr make_typed_outlet(symbol ty, Args&&... args)
        {
            return make_port<outlet>(ty, std::forward<Args>(args)...);
        }

        template <typename... Args>
        outlet_ptr make_signal_outlet(Args... args)
        {
            make_port<outlet>(sym::signal, std::forward<Args>(args)...);
        }

        template <typename... Args>
        inlet_ptr make_signal_inlet(Args... args)
        {
            return make_port<inlet>(sym::signal, std::forward<Args>(args)...);
        }

        template <typename... Args>
        outlet_ptr make_mc_outlet(Args... args)
        {
            return make_port<outlet>(
                sym::multichannelsignal, std::forward<Args>(args)...);
        }

        template <typename... Args>
        inlet_ptr make_mc_inlet(Args... args)
        {
            return make_port<inlet>(
                sym::multichannelsignal, std::forward<Args>(args)...);
        }
        
        template <typename... Args>
        outlet_ptr make_mc_outlet(long channelcount, Args... args)
        {
            auto p = make_port<outlet>(
                sym::multichannelsignal, std::forward<Args>(args)...);
            
            p->signale_count(channelcount);
            
            return p;
        }

        template <typename... Args>
        inlet_ptr make_mc_inlet(long channelcount, Args... args)
        {
            auto p = make_port<inlet>(
                sym::multichannelsignal, std::forward<Args>(args)...);

            p->signal_count(channelcount);

            return p;
        }

        template <typename port_type, typename... Args>
        std::shared_ptr<port_type> make_port(symbol ty, Args... args)
        {
            auto port =
                std::make_shared<port_type>(std::forward<Args>(args)...);

            port->type(ty);
            
            lmw_internal_assign(port);

            return port;
        }

        const std::vector<inlet_ptr>& inlets() const noexcept
        {
            return m_inlets;
        }
        
        std::vector<inlet_ptr>& inlets() noexcept
        {
            return m_inlets;
        }
        
        const std::vector<outlet_ptr>& outlets() const noexcept
        {
            return m_outlets;
        }
        
        std::vector<outlet_ptr>& outlets() noexcept
        {
            return m_outlets;
        }

        void lmw_internal_finalize()
        {
            auto sort_signals = [](const auto& lhs, const auto& rhs) {
                return lhs->signal() && !rhs->signal();
            };

            std::sort(m_inlets.begin(), m_inlets.end(), sort_signals);
            std::sort(m_outlets.begin(), m_outlets.end(), sort_signals);

            for (auto it = m_inlets.rbegin(); it != m_inlets.rend(); ++it)
                (*it)->lmw_internal_create(this,
                                           std::distance(m_inlets.rbegin(), it),
                                           m_inlets.size());

            for (auto it = m_outlets.rbegin(); it != m_outlets.rend(); ++it)
                (*it)->lmw_internal_create(
                    this, std::distance(m_outlets.rbegin(), it),
                    m_inlets.size());
            
            if(mc()){
                mspflag(c74::max::Z_MC_INLETS);
                mspflag(c74::max::Z_NO_INPLACE);
            }
        }

        template <typename... Args>
        void post(const char* msg, Args... args)
        {
            c74::max::object_post(native_handle(), msg, args...);
        }

        template <typename... Args>
        void error(const char* msg, Args... args)
        {
            c74::max::object_error(native_handle(), msg, args...);
        }

        template <typename... Args>
        void warn(const char* msg, Args... args)
        {
            c74::max::object_warn(native_handle(), msg, args...);
        }
        
        virtual void prepare(double srate, long max_vsize)
        {
        }
        
        void clear_mspflags() noexcept
        {
            m_mspflags = 0;
        }
        
        void mspflag(short flag) noexcept
        {
            m_mspflags |= flag;
        }
        
        short mspflags() const noexcept
        {
            return m_mspflags;
        }

      protected:
        
        console_stream<default_console_stream> console;
        console_stream<warning_console_stream> console_warn;
        console_stream<error_console_stream> console_error;

      private:
        
        template <typename PortArr>
        const char* get_port_description(const PortArr& p, long index)
        {
            if(LMW_UNLIKELY(index >= p.size()))
                return "unknown";
            
            return p[index]->description();
        }
        
        void lmw_internal_assign(message* msg)
        {
            m_messages.insert({msg->name(), msg});
        }
        
        void lmw_internal_assign(inlet_ptr inlet)
        {
            m_inlets.push_back(inlet);
        }
        
        void lmw_internal_assign(outlet_ptr outlet)
        {
            m_outlets.push_back(outlet);
        }

        void lmw_internal_prepare(c74::max::t_object* instance_ptr)
        {
            t_obj_instance_ptr = instance_ptr;

            console.lmw_internal_prepare(t_obj_instance_ptr);
            console_warn.lmw_internal_prepare(t_obj_instance_ptr);
            console_error.lmw_internal_prepare(t_obj_instance_ptr);
        }

        short m_mspflags = 0;
        c74::max::t_object*                         t_obj_instance_ptr = nullptr;
        std::unordered_map<std::string, message*>   m_messages;
        std::vector<std::shared_ptr<inlet>>         m_inlets;
        std::vector<std::shared_ptr<outlet>>        m_outlets;
    };

    template <typename user_class>
    class max_class : public max_class_base {
      public:
        virtual ~max_class()
        {
        }

      private:
    };
}