#include "stdafx.h"
#include "ImageStitch.h"

////////////////////////////////////////////////////////////////////////////

int getOffset(int i, int j, int nrows, int ncols) {
    return j * nrows + i;
}
////////////////////////////////////////////////////////////////////////////

int compute_exact_homography(const vector<PCorrespond> &pointset,
        vector<double> &H) {
    assert(pointset.size() == 4);

    // Calculate  [p2] = {H}[p1]
    int a = 0;
    int b = 1;
    int c = 2;
    int d = 3;
    int e = 4;
    int f = 5;
    int g = 6;
    int h = 7;

    double A[64], B[8], copyA[64], copyB[8];

    int irow, icol, offset;
    double x1, y1, x2, y2;

    for (int i = 0; i < 64; i++) A[i] = 0.0;

    for (int i = 0; i < 4; i++) {
        x1 = pointset[i].p1[0];
        y1 = pointset[i].p1[1];
        x2 = pointset[i].p2[0];
        y2 = pointset[i].p2[1];

        irow = 2 * i;
        icol = a;
        offset = getOffset(irow, icol, 8, 8);
        A[offset] = x1;

        icol = b;
        offset = getOffset(irow, icol, 8, 8);
        A[offset] = y1;

        icol = c;
        offset = getOffset(irow, icol, 8, 8);
        A[offset] = 1;

        icol = g;
        offset = getOffset(irow, icol, 8, 8);
        A[offset] = -1.0 * x1*x2;

        icol = h;
        offset = getOffset(irow, icol, 8, 8);
        A[offset] = -1.0 * y1*x2;

        B[irow] = x2;

        irow = 2 * i + 1;
        icol = d;
        offset = getOffset(irow, icol, 8, 8);
        A[offset] = x1;

        icol = e;
        offset = getOffset(irow, icol, 8, 8);
        A[offset] = y1;

        icol = f;
        offset = getOffset(irow, icol, 8, 8);
        A[offset] = 1;

        icol = g;
        offset = getOffset(irow, icol, 8, 8);
        A[offset] = -1.0 * x1*y2;

        icol = h;
        offset = getOffset(irow, icol, 8, 8);
        A[offset] = -1.0 * y1*y2;

        B[irow] = y2;
    }

    for (int i = 0; i < 64; i++) copyA[i] = A[i];
    for (int i = 0; i < 8; i++) copyB[i] = B[i];

    int n = 8;
    int nrhs = 1;
    int lda = max(1, n);
    int ldb = 8;
    int ipiv[8]; // not used ???????????????????????????????????????????????????????
    int info;

//  dgesv(&n, &nrhs, A, &lda, ipiv, B, &ldb, &info);

    H.resize(9);
    for (int i = 0; i < 8; i++)
        H[i] = B[i];

    H[8] = 1.0;
    if (info) {   // ????????????????????????????????????????????????????
        cout << " Failed Matrix : " << endl;
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                offset = getOffset(i, j, 8, 8);
                cout << fixed << copyA[offset] << " ";
            }
            cout << endl;
        }

        cout << " Failed B : " << endl;
        for (int i = 0; i < 8; i++)
            cout << copyB[i] << endl;

        cout << " Failed X : " << endl;
        for (int i = 0; i < 8; i++)
            cout << B[i] << endl;


        exit(0);
    }
    return info;
}

////////////////////////////////////////////////////////////////////////////

/*int compute_ls_homography(const vector<PCorrespond> &pset1,
        vector<double> &H) {
}*/

////////////////////////////////////////////////////////////////////////////

int compute_homography(const vector<PCorrespond> &pointset,
        vector<double> &H) {
    assert(pointset.size() == 4);
    int info = compute_exact_homography(pointset, H);
    return info;
}

////////////////////////////////////////////////////////////////////////////

void collect_inliners(const vector<PCorrespond> &pointset,
        const Point2F &trans, vector<int> &inliners) 
{
    inliners.clear();

    int nSize = pointset.size();

    for (int i = 0; i < nSize; i++) {
        double x1 = pointset[i].p1[0] + trans[0];
        double y1 = pointset[i].p1[1] + trans[1];

        double x2 = pointset[i].p2[0];
        double y2 = pointset[i].p2[1];

        double dx = fabs(x2 - x1);
        double dy = fabs(y2 - y1);
        if (max(dx, dy) < 2) inliners.push_back(i);
    }
}

////////////////////////////////////////////////////////////////////////////

int random_index(int minValue, int maxValue) {
    return rand() % maxValue;
}
////////////////////////////////////////////////////////////////////////////

void ransac_translation(const vector<PCorrespond> &pmatch,
        Point2F &trans) {
    double d = 5; // maximum pixel movement;
    double p = 0.75; // probability of real inliners
    double P = 0.99; // Probability of 1 success after k trials
    vector<double> H;

    int N = pmatch.size();
    int n = 1;

    int numSteps = (int)(log(1 - P) / log(1 - pow(p, n)));

    vector<PCorrespond> sample;
    sample.resize(n);

    int maxConsensus = 0;

    vector<int> inliners, consensusSet;
    int index, iter = 0;

    while (iter != numSteps) {
        index = random_index(0, N - 1);
        trans[0] = pmatch[index].p2[0] - pmatch[index].p1[0];
        trans[1] = pmatch[index].p2[1] - pmatch[index].p1[1];
        collect_inliners(pmatch, trans, inliners);
        if (inliners.size() > maxConsensus) {
            consensusSet = inliners;
            maxConsensus = inliners.size();
        }
        iter++;
    }

    cout << " Total Set Size " << pmatch.size() << " Consensus " << consensusSet.size() << endl;

    trans[0] = 0.0;
    trans[1] = 0.0;
    for (int i = 0; i < consensusSet.size(); i++) {
        index = consensusSet[i];
        trans[0] += pmatch[index].p1[0] - pmatch[index].p2[0];
        trans[1] += pmatch[index].p1[1] - pmatch[index].p2[1];
    }

    trans[0] /= (consensusSet.size());
    trans[1] /= (consensusSet.size());
}
////////////////////////////////////////////////////////////////////////////
