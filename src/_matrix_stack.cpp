#pragma once
struct matrix_stack_t
{
    enum { stack_capacity = 1024 };
    int stack_index;
    vdbMat4 data[stack_capacity];

    void Reset()
    {
        stack_index = 0;
        LoadIdentity();
    }

    vdbMat4 Top()
    {
        assert(stack_index >= 0 && stack_index < stack_capacity);
        return data[stack_index];
    }

    void Push()
    {
        assert(stack_index + 1 < stack_capacity);
        stack_index++;
        data[stack_index] = data[stack_index-1];
    }

    void Pop()
    {
        assert(stack_index > 0);
        stack_index--;
    }

    void LoadIdentity()
    {
        assert(stack_index >= 0 && stack_index < stack_capacity);
        data[stack_index] = vdbMatIdentity();
    }

    void Load(vdbMat4 m)
    {
        assert(stack_index >= 0 && stack_index < stack_capacity);
        data[stack_index] = m;
    }

    void Multiply(vdbMat4 m)
    {
        assert(stack_index >= 0 && stack_index < stack_capacity);
        data[stack_index] = vdbMul4x4(data[stack_index], m);
    }
};
