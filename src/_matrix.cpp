struct vdbMat4
{
    float data[4*4];
    #if defined(VDB_MATRIX_COLUMN_MAJOR)
    float &at(int row, int col) { return data[col + row*4]; }
    #else
    float &at(int row, int col) { return data[row + col*4]; }
    #endif
};

static vdbMat4 vdbMul4x4(vdbMat4 a, vdbMat4 b)
{
    vdbMat4 c = {0};
    for (int row = 0; row < 4; row++)
    for (int col = 0; col < 4; col++)
    {
        c.at(row,col) = 0.0f;
        for (int i = 0; i < 4; i++)
            c.at(row,col) += a.at(row,i)*b.at(i,col);
    }
    return c;
}

static vdbVec4 vdbMul4x1(vdbMat4 a, vdbVec4 b)
{
    vdbVec4 c(0.0f,0.0f,0.0f,0.0f);
    c.x = b.x*a.at(0,0) + b.y*a.at(0,1) + b.z*a.at(0,2) + b.w*a.at(0,3);
    c.y = b.x*a.at(1,0) + b.y*a.at(1,1) + b.z*a.at(1,2) + b.w*a.at(1,3);
    c.z = b.x*a.at(2,0) + b.y*a.at(2,1) + b.z*a.at(2,2) + b.w*a.at(2,3);
    c.w = b.x*a.at(3,0) + b.y*a.at(3,1) + b.z*a.at(3,2) + b.w*a.at(3,3);
    return c;
}

static vdbVec4 vdbMulSE3Inverse(vdbMat4 a, vdbVec4 b)
{
    vdbVec4 c(b.x-a.at(0,3)*b.w, b.y-a.at(1,3)*b.w, b.z-a.at(2,3)*b.w, b.w);
    vdbVec4 d(0.0f,0.0f,0.0f,0.0f);
    d.x = c.x*a.at(0,0) + c.y*a.at(1,0) + c.z*a.at(2,0);
    d.y = c.x*a.at(0,1) + c.y*a.at(1,1) + c.z*a.at(2,1);
    d.z = c.x*a.at(0,2) + c.y*a.at(1,2) + c.z*a.at(2,2);
    d.w = c.w;
    return d;
}

static vdbMat4 vdbMatIdentity()
{
    vdbMat4 result = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    return result;
}
