//#include <termios.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <functional>
//#include <unistd.h>

#include <stdio.h>

#include <utility>

#include "AVLTree.h"
#include "PageFrameAllocator.h"

#define NUM_ITEMS 1000

int testItems[NUM_ITEMS]; // = { 9, 0, 1, 2, 3, 4, 5, 6, 7, 8 };
int testValues[NUM_ITEMS];

int RandTo(int max)
{
    double d = (double)rand() / (double)RAND_MAX;
    return (int)(d * (max - 1));
}

void InitTestValues()
{
    //size_t max = 0;

    for (int i = 0; i < NUM_ITEMS; ++i)
    {
        testItems[i] = i;

        int v = RandTo(4) + 1;

        testValues[i] = 4096 * v;
        //max += testValues[i];
    }

    //printf("To allocate: %ul\r\n", max);
}

void ShuffleItems()
{
    for (int i = 0; i < NUM_ITEMS; ++i)
    {
        int v = RandTo(NUM_ITEMS);
        std::swap(testItems[i], testItems[v]);
    }
}

void TestFooTest()
{
    //InitTestValues();

    //ShuffleItems();

#if 0
    IntTree test;

    for (int i = 0; i < NUM_ITEMS; ++i)
    {
        int value = testItems[i];

        test.Insert(value, value);
    }

    if (test.GetHeight() == -1)
    {
        printf("TREE INBALANCED!\r\n");
        test.Print();
        return 1;
    }

    //test.Print();
    Shuffle();

    for (int i = 0; i < NUM_ITEMS; ++i)
    {
        int value = testItems[i];

        if (!test.Remove(value))
        {
            printf("Index: %d\r\n", i);

            printf("Failure removing item: %d\r\n", value);

            if (i > 0)
                printf("Previous item: %d\r\n", testItems[i - 1]);

            test.CheckTree();
            int h = test.GetHeight();

            if (h == -1)
                printf("Tree is inbalanced!\r\n");
        }
    }

    if (test.Count() != 0)
    {
        printf("The tree is not empty!!!\r\n");
    }

    // Tree should be empty
    test.Print();
#endif

#if 0
    // Simulate 32 MB
    PageFrameAllocator test(32ull * 1024 * 1024);

    PageBlock pb = test.Aquire(0x000A0000, 65536);

    printf("Addr: 0x%08X - %d\r\n", pb.Start, pb.Size);

#if 0
    PageBlock blocks[NUM_ITEMS];

    for (int i = 0; i < NUM_ITEMS; ++i)
    {
        int index = testItems[i];

        blocks[index] = test.Allocate(testValues[index]);

        if (!blocks[index])
        {
            printf("Out of memory!?\r\n");
            return 1;
        }
    }

    ShuffleItems();

    for (int i = 0; i < NUM_ITEMS; ++i)
    {
        int index = testItems[i];

        test.Release(std::move(blocks[index]));
    }

    if (!test.CheckAllFree())
    {
        printf("Not all items were released!\r\n");
        return 1;
    }
#endif

#endif
}

/**
 * @brief Wraps a reference and calls a callback if modified.
 * @tparam T The type of reference to wrap
 */
template <typename T>
class watch
{
public:
    typedef std::function<void()> callback;

private:
    T &m_value;
    callback m_callback;

public:
    /* constructor */ watch(T &v, callback cb)
        : m_value(v)
        , m_callback(cb)
    { }

    /* constructor */ watch(const watch &w)
        : m_value(w.m_value)
        , m_callback(w.m_callback)
    { }

    /* constructor */ watch(watch &&w)
        : m_value(std::move(w.m_value))
        , m_callback(std::move(w.m_callback))
    { }

    operator T() const { return m_value; }

    const T &operator =(const T &nv)
    {
        m_value = nv;
        m_callback();
        return m_value;
    }
};

class Bar
{
private:
    uint8_t *m_data;

public:
    Bar() { m_data = new uint8_t[16]; }
    virtual ~Bar() { delete[] m_data; }

    uint8_t operator[](int index) const
    {
        return m_data[index];
    }

    uint8_t &operator[](int index)
    {

        return m_data[index];
    }
};

//auto cb = [] () { printf("Modified\r\n"); };
// watch<uint8_t>(m_data[index], cb);

class DerpArchive
{
private:
    bool m_write;

public:
    DerpArchive(bool write) : m_write(write) { }

    template <typename T>
    void operation(const std::string &name, T &val)
    {
        if (m_write)
            std::cout << "WRITE: " << name << ":" << val << std::endl;
        else
            std::cout << "READ: " << name << ":" << val << std::endl;
    }
};


namespace cfg
{
    class context;

    static const char PATH_SEPERATOR = '/';

    /****************************************************************/

    template <typename T>
    struct mapper
    {
        typedef typename std::decay<T>::type type;

        bool to(type &value, context &ctx) { return false; };
    };

    /****************************************************************/

    class source
    {
    private:
        std::string m_root;

    protected:
        /* constructor */ source(const std::string_view &root) : m_root(root) { }

        friend class context;

#define DECLARE_OPS(T__) \
            virtual bool writeValue(const std::string &path, const T__ &value) = 0; \
            virtual bool readValue(const std::string &path, T__ &value) = 0;

        //DECLARE_OPS(bool)
        DECLARE_OPS(int)
        //DECLARE_OPS(unsigned int)
        //DECLARE_OPS(float)
        //DECLARE_OPS(double)
        DECLARE_OPS(std::string)

#undef DECLARE_OPS

        std::shared_ptr<context> createContext(bool reading);

    public:
        virtual ~source() { }

        template <typename T>
        bool read(const std::string &path, T &value);

        template <typename T>
        bool write(const std::string &path, T &value);

    };

    /****************************************************************/

    class context
    {
    private:
        context(const context &) = delete;
        context(context &&) = delete;

        const context &operator =(const context &) = delete;
        const context &operator =(context &&) = delete;

    private:
        source *m_source;
        bool m_reading;

        std::string m_currentPath;

        std::string pushPath(const std::string &path)
        {
            return m_currentPath + PATH_SEPERATOR + path;
        }

    protected:
        friend class source;

        context(source *src, const std::string_view &path, bool reading)
            : m_source(src)
            , m_reading(reading)
            , m_currentPath(path)
        { }

    public:
        virtual ~context() { }

        template <typename T>
        bool map(const std::string &path, T &value)
        {
            std::string lastPath = m_currentPath;
            m_currentPath = pushPath(path);

            mapper<T> map;
            return map.to(value, *this);

            m_currentPath = lastPath;
        }

        template <>
        bool map(const std::string &path, int &value)
        {
            std::string fullPath = pushPath(path);

            return m_reading ?
                m_source->readValue(fullPath, value) :
                m_source->writeValue(fullPath, value);
        }

        template <>
        bool map(const std::string &path, std::string &value)
        {
            std::string fullPath = pushPath(path);

            return m_reading ?
                m_source->readValue(fullPath, value) :
                m_source->writeValue(fullPath, value);
        }
    };

    /****************************************************************/

    std::shared_ptr<context> source::createContext(bool reading)
    {
        return std::shared_ptr<context>(new context(this, m_root, reading));
    }

    /****************************************************************/

    template <typename T>
    bool source::read(const std::string &path, T &value)
    {
        auto ctx = createContext(true);
        return ctx->map(path, value);
    }

    template <typename T>
    bool source::write(const std::string &path, T &value)
    {
        auto ctx = createContext(false);
        return ctx->map(path, value);
    }

    /****************************************************************/
}


struct TestData
{
    int id;
    std::string name;
};


template<>
struct cfg::mapper<TestData> //: public mapper_base
{
    bool to(TestData &cfg, cfg::context &ctx) const
    {
        return
            ctx.map("id", cfg.id) &&
            ctx.map("name", cfg.name)
            ;
    }
};

class DummySource : public cfg::source
{
protected:
    virtual bool writeValue(const std::string &path, const int &value) override
    {
        printf("int '%s' write\r\n", path.c_str());
        return true;
    }

    virtual bool readValue(const std::string &path, int &value) override
    {
        printf("int '%s' read\r\n", path.c_str());
        return true;
    }

    virtual bool writeValue(const std::string &path, const std::string &value) override
    {
        printf("str '%s' write\r\n", path.c_str());
        return true;
    }

    virtual bool readValue(const std::string &path, std::string &value) override
    {
        printf("str '%s' read\r\n", path.c_str());
        return true;
    }

public:
    DummySource(const std::string_view &root) : source(root) { }
};

// Simulating our 4bit multiply lookup table.
int mulLookup(int a, int b)
{
    return (a * b) & 0xFF;
}

void mulTest()
{
    int a = 0x34;
    int b = 0x43;
    //int b = 0x03;

    int test = a * b;
    int res = 0;

    // Simulating 8 bit multiply with 4 bit slices

    for (int i = 0; i < 2; ++i)
    {
        int al = (a & 0xF0) >> 4;

        a <<= 4;
        res <<= 4;

        int bt = b;
        int bsum = 0;

        for (int j = 0; j < 2; ++j)
        {
            int bl = (bt & 0xF0) >> 4;

            bt <<= 4;
            bsum <<= 4;

            bsum += mulLookup(al, bl);
        }

        res += bsum;
    }
}

void bitMulTest()
{
    int a = 0x34;
    int b = 0x43;

    int test = a * b;
    int res = 0;

    for (int i = 0; i < 8; ++i)
    {
        bool t = (a & 0x80) != 0;

        a <<= 1;
        res <<= 1;

        if (t)
            res += b;
    }
}

int foo()
{
}

int main()
{
    mulTest();
    //bitMulTest();

    /*
    TestData d
    {
        .id = 5,
        .name = "Bob"
    };

    DummySource source("/AppData");

    source.read("TestData", d);

    source.write("TestData", d);
    */

    /*
    DerpArchive arch(true);

    arch.operation("id", d.id);
    arch.operation("name", d.name);
    */

    //Bar bar;

    //int i = bar[10];
    //bar[10] = 20;

    return 0;
}
