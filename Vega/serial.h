#pragma once
#include <Eigen/Core>
#include <boost\asio.hpp>
#include <chrono>
#include <mutex>
#include <regex>
#include <string>
#include <thread>
using namespace boost::asio;

constexpr size_t LED_WIDTH    = 8;
constexpr size_t LED_LENGTH   = 8;
const std::string serial_head = "%";
const std::string serial_tail = "#";
class serial
{
public:
    serial(const std::string COM) : sp(iosev, COM)
    {
        sp.set_option(serial_port::baud_rate(14400));
        sp.set_option(serial_port::flow_control(serial_port::flow_control::none));
        sp.set_option(serial_port::parity(serial_port::parity::none));
        sp.set_option(serial_port::stop_bits(serial_port::stop_bits::one));
        sp.set_option(serial_port::character_size(8));
        thread_mux.test_and_set();
        start_receive();
        is_ready.lock();
        is_active.test_and_set();
        //ar = Eigen::Matrix<uint32_t, LED_LENGTH, LED_WIDTH>::Zero();
    }
    ~serial()
    {
        thread_mux.clear();
        sp.close();
    }
    bool send_ar(Eigen::Matrix<uint32_t, LED_LENGTH, LED_WIDTH> arr)
    {
        sp.write_some(buffer(serialize(arr)));
        return is_success();
    }
    bool send_active()
    {
        this->send("LED active");
        is_active.clear();
        return is_success();
    }
    bool is_Active()
    {
        return is_active.test_and_set();
    }
    bool send_motor(int motor, int index)
    {
        std::string send = "MOTOR" + std::to_string(motor) + ' ' + std::to_string(index * 60);
        this->send(send);
        return is_success();
    }
    size_t send(const std::string &data)
    {

        return sp.write_some(buffer(serial_head + data + serial_tail));
    }

    bool is_success()
    {
        std::string buf;
        return get_receive_package(buf) && buf == "Done.";
    }
    bool get_receive_package(std::string &ret)
    {
        if (is_ready.try_lock_for(std::chrono::milliseconds(1500)))
        {
            mut.lock();
            ret = read;
            read.clear();
            mut.unlock();
            return true;
        }
        else
            return false;
    }

private:
    void start_receive()
    {
        std::thread t([&]() {
            while (thread_mux.test_and_set())
            {
                std::smatch sm;
                std::string buf = "123456"; //! ????
                std::string rec;
                do
                {
                    while (!sp.read_some(buffer(buf)))
                        ;
                    rec += buf;
                } while (!std::regex_search(rec, sm, std::regex(serial_head + "(.*)\\" + serial_tail)));
                if (mut.try_lock())
                {
                    rec.clear();
                    read = sm[1];
                    mut.unlock();
                    is_ready.unlock();
                }
            }
            thread_mux.clear();
        });
        t.detach();
    }

private:
    io_service iosev;
    serial_port sp;

    std::mutex mut;
    std::string read;
    std::timed_mutex is_ready;
    //Eigen::Matrix<uint32_t, LED_LENGTH, LED_WIDTH> ar;

    std::atomic_flag thread_mux;

    std::atomic_flag is_active;

private:
    std::string serialize(Eigen::Matrix<uint32_t, LED_LENGTH, LED_WIDTH> ar)
    {
        std::stringstream buf;
        buf << serial_head << "LED ";
        buf.unsetf(std::stringstream::dec);
        buf.setf(std::stringstream::hex);
        buf.setf(std::stringstream::uppercase);
        for (size_t i = 0, j = 0; i < ar.rows(); ++i)
        {
            for (; j < ar.cols() && j >= 0; i % 2 ? --j : ++j)
            //for (size_t j = 0; j < ar.cols(); ++j)
            {
                buf << ar(i, j) << ' ';
            }
            i % 2 ? ++j : --j;
        }
        buf << serial_tail;
        return buf.str();
    };
};
uint32_t rgb(uint8_t r, uint8_t g, uint8_t b) { return g << 16 | r << 8 | b; };
uint32_t white(uint8_t w) { return w << 16 | w << 8 | w; };