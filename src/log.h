#pragma once
#include <vector>
typedef int log_type_t;
enum log_type_
{
    log_type_group = 0,
    log_type_scalar,
    log_type_matrix
};

struct log_t
{
    const char *label;
    log_t *parent;
    log_type_t type;
    std::vector<log_t*> children;
    std::vector<float> data;
    int rows, columns; // for matrix types
                       // note: matrix data is always column-major
};

struct logs_t
{
    log_t root;
    log_t *curr;
    logs_t()
    {
        root.type = log_type_group;
        root.label = NULL;
        root.parent = NULL;
        curr = &root;
    }

    void Push(const char *label)
    {
        assert(curr);

        for (size_t i = 0; i < curr->children.size(); i++)
        {
            if (curr->children[i]->label &&
                strcmp(curr->children[i]->label, label) == 0)
            {
                curr = curr->children[i];
                return;
            }
        }

        log_t *child = new log_t;
        assert(child);
        child->label = label;
        child->type = log_type_group;
        child->parent = curr;

        curr->children.push_back(child);
        curr = child;
    }

    void Push()
    {
        assert(curr);

        log_t *child = new log_t;
        assert(child);
        child->label = NULL;
        child->type = log_type_group;
        child->parent = curr;

        curr->children.push_back(child);
        curr = child;
    }

    void Pop()
    {
        assert(curr);
        assert(curr->parent && "Mismatched vdbLogPush/vdbLogPop pair");
        curr = curr->parent;
    }

    bool CompareUnterminatedString(const char *s1_begin, const char *s1_end, const char *s2)
    {
        const char *c1 = s1_begin;
        const char *c2 = s2;
        while (c1 < s1_end && *c2)
        {
            if (*c1 != *c2)
                return false;
            c1++;
            c2++;
        }
        if (c1 == s1_end && *c2 == '\0')
            return true;
        return false;
    }

    log_t *Find(const char *str)
    {
        if (!str)
            return NULL;
        log_t *l = &root;
        const char *c = str;
        while (*c)
        {
            if (*c != '/')
                return NULL;

            c++;

            // term is an index
            if ((*c >= '0' && *c <= '9') || *c == '-')
            {
                char *end;
                int i = strtol(c, &end, 0);
                if (i < 0)
                    i += (int)l->children.size();
                if (i < 0 || i >= (int)l->children.size())
                    return NULL;
                l = l->children[i];
                c = end - 1;
            }
            // term is a label
            else
            {
                const char *end = c;
                while (*end && *end != '/')
                    end++;

                if (end == c)
                    return NULL;

                int match = -1;
                for (int i = 0; i < (int)l->children.size(); i++)
                {
                    if (!l->children[i]->label)
                        continue;
                    if (CompareUnterminatedString(c, end, l->children[i]->label))
                    {
                        match = i;
                        break;
                    }
                }
                if (match < 0)
                    return NULL;
                l = l->children[match];
                c = end - 1;
            }
            c++;
        }
        return l;
    }

    log_t *GetLog(const char *label, log_type_t type)
    {
        for (size_t i = 0; i < curr->children.size(); i++)
        {
            if (curr->children[i]->type == type &&
                strcmp(curr->children[i]->label, label) == 0)
                return curr->children[i];
        }

        log_t *l = new log_t;
        assert(l);
        l->label = label;
        l->type = type;
        l->parent = curr;
        curr->children.push_back(l);
        return l;
    }

    void Scalar(const char *label, float x)
    {
        log_t *l = GetLog(label, log_type_scalar);
        l->data.push_back(x);
    }

    void Matrix(const char *label, float *x, int rows, int columns)
    {
        log_t *l = GetLog(label, log_type_matrix);
        l->rows = rows;
        l->columns = columns;
        for (int col = 0; col < columns; col++)
        for (int row = 0; row < rows; row++)
            l->data.push_back(x[row + col*rows]);
    }

    void Matrix_RowMaj(const char *label, float *x, int rows, int columns)
    {
        log_t *l = GetLog(label, log_type_matrix);
        l->rows = rows;
        l->columns = columns;
        for (int col = 0; col < columns; col++)
        for (int row = 0; row < rows; row++)
            l->data.push_back(x[col + row*columns]);
    }

    void Vector(const char *label, float *x, int elements)
    {
        Matrix(label, x, elements, 1);
    }

    void _Dump(FILE *f, log_t *l, int indent_level)
    {
        #define indent for (int i = 0; i < indent_level; i++) fprintf(f, "\t");
        if (l->type == log_type_group)
        {
            if (l->label)
            {
                indent
                fprintf(f, "\"%s\":\n", l->label);
            }
            const char *open = "{\n";
            const char *close = "}";
            if (l->children.size() >= 1 && l->children[0]->label == NULL)
            {
                open = "[\n";
                close = "]";
            }
            indent
            fprintf(f, open);
            _Dump(f, l->children[0], indent_level + 1);
            for (size_t i = 1; i < l->children.size(); i++)
            {
                fprintf(f, ",\n");
                _Dump(f, l->children[i], indent_level + 1);
            }
            fprintf(f, "\n");
            indent
            fprintf(f, close);
        }
        else
        {
            if (l->type == log_type_scalar)
            {
                indent
                fprintf(f, "\"%s\": ", l->label);
                if (l->data.size() == 1)
                {
                    fprintf(f, "%g", l->data[0]);
                }
                else
                {
                    fprintf(f, "[%g", l->data[0]);
                    for (size_t i = 1; i < l->data.size(); i++)
                        fprintf(f, ", %g", l->data[i]);
                    fprintf(f, "]");
                }
            }
            else if (l->type == log_type_matrix)
            {
                int n = l->rows*l->columns;
                size_t count = l->data.size() / n;
                indent
                fprintf(f, "\"%s\": ", l->label);
                if (count == 1)
                {
                    fprintf(f, "[%g", l->data[0]);
                    for (int i = 1; i < n; i++)
                        fprintf(f, ", %g", l->data[i]);
                    fprintf(f, "]");
                }
                else
                {
                    size_t j = 0;
                    fprintf(f, "[[%g", l->data[j++]);
                    for (int i = 1; i < n; i++)
                        fprintf(f, ", %g", l->data[j++]);
                    fprintf(f, "]");
                    for (size_t i = 1; i < count; i++)
                    {
                        fprintf(f, ", [%g", l->data[j++]);
                        for (int i = 1; i < n; i++)
                            fprintf(f, ", %g", l->data[j++]);
                        fprintf(f, "]");
                    }
                    fprintf(f, "]");
                }
            }
        }
        #undef indent
    }

    void Dump(const char *filename)
    {
        FILE *f = fopen(filename, "w+");
        _Dump(f, &root, 0);
    }
};

static logs_t logs;

void vdbLogPush(const char *label) { logs.Push(label); }
void vdbLogPush() { logs.Push(); }
void vdbLogPop() { logs.Pop(); }
void vdbLogScalar(const char *label, float x) { logs.Scalar(label, x); }
void vdbLogMatrix(const char *label, float *x, int rows, int columns) { logs.Matrix(label, x, rows, columns); }
void vdbLogMatrix_RowMaj(const char *label, float *x, int rows, int columns) { logs.Matrix_RowMaj(label, x, rows, columns); }
void vdbLogVector(const char *label, float *x, int elements) { logs.Vector(label, x, elements); }
void vdbLogDump(const char *filename) { logs.Dump(filename); }
