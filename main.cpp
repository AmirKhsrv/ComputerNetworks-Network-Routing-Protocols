#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <map>

using namespace std;

class Router;

struct Route{
    Router* destination;
    Router* nextHop; 
    int cost;
    vector<Router*> path;
};
typedef struct Route Route;
typedef vector<Route> Routes;

struct LSP{
    Router* source;
    map<Router*, int> adjacents;
    vector<Router*> path;
};
typedef struct LSP LSP;
typedef vector<LSP> LSPs;

struct Spt{
    Router* destination;
    int distance;
};
typedef struct Spt Spt;
typedef vector<Spt> SptSet;

///////////////////////////////////////////////// ROUTER /////////////////////////////////////////////////

class Router
{
public:

    Router(int _id) {id = _id;}
    int getId() {return id;}
    void addRoute(Route route) {routes.push_back(route);}
    Routes getRoutes() {return routes;}
    bool exchangeRoutes(Router* router, int weight, Routes _routes);
    void deleteRoutes() {routes.clear();}
    void deleteLsps() {lsps.clear();}
    bool exchangeLsps(LSPs _lsps);
    LSPs getLsps() {return lsps;}
    void addLsp(LSP lsp) {lsps.push_back(lsp);}

private:

    LSPs lsps;
    Routes routes;
    int id;
};

bool Router::exchangeLsps(LSPs _lsps)
{
    bool changed = false;
    for (int i = 0; i < _lsps.size(); i++)
    {
        bool found = false;
        for (int j = 0; j < lsps.size(); j++)
            if (_lsps[i].source->getId() == lsps[j].source->getId())
                found = true;
        if (!found)
        {
            lsps.push_back(_lsps[i]);
            changed = true;
        }
    }
    return changed;
}

bool Router::exchangeRoutes(Router* router, int weight, Routes _routes)
{
    bool changed = false;
    for (int i = 0; i < routes.size(); i++)
        for (int j = 0; j < _routes.size(); j++)
            if (routes[i].destination == _routes[j].destination && weight
                + _routes[i].cost < routes[i].cost)
            {
                routes[i].cost = _routes[i].cost + weight;
                routes[i].nextHop = router;
                _routes[i].path.insert(_routes[i].path.begin(), this);
                routes[i].path = _routes[i].path;
                changed = true;
            }
    return changed;
}

///////////////////////////////////////////////// LINK /////////////////////////////////////////////////

class Link
{
public:

    Link(Router* r1, Router* r2, int w) {router1 = r1; router2 = r2; weight = w;}
    Router* getFirstRouter() {return router1;}
    Router* getSecondRouter() {return router2;}
    int getWeight() {return weight;}
    void setNewWeight(int _weight) {weight = _weight;}

private: 

    Router* router1;
    Router* router2;
    int weight;
};

typedef std::vector<Router*> Routers;
typedef std::vector<Link*> Links;

///////////////////////////////////////////////// NETWORK /////////////////////////////////////////////////

class Network
{
public:

    Network(){}
    Router* getRouter(int routerId);
    void addLink(Router* r1, Router* r2, int weight);
    void sortRoutersById();
    vector<int> getRoutersIds();
    int getLinkWeight(int routerId1, int routerId2);
    void doDvrpAlgorithm();
    Routes getDvrpResults(int src);
    void doLsrpAlgorithm();
    Routes getLsrpResults(int src);
    SptSet getSptSet(int src);
    void modifyLink(int routerId1, int routerId2, int weight);
    void removeLink(int routerId1, int routerId2);

private:

    Routers routers;
    Links links;
};

void Network::removeLink(int routerId1, int routerId2)
{
    for (int i = 0; i < links.size(); i++)
        if (links[i]->getFirstRouter()->getId() == routerId1 && links[i]->getSecondRouter()->getId() == routerId2)
            links.erase(links.begin() + i);
}

void Network::modifyLink(int routerId1, int routerId2, int weight)
{
    bool modified = false;
    for (int i = 0; i < links.size(); i++)
        if (links[i]->getFirstRouter()->getId() == routerId1 && links[i]->getSecondRouter()->getId() == routerId2)
        {
            links[i]->setNewWeight(weight);
            modified = true;
        }
    if (!modified)
        addLink(getRouter(routerId1), getRouter(routerId2), weight);
    
}

SptSet Network::getSptSet(int src)
{
    SptSet sptSet;
    sortRoutersById();
    for (int i = 0; i < routers.size(); i++)
    {
        Spt spt;
        spt.destination = routers[i];
        int weight = getLinkWeight(routers[i]->getId(), src);
        if (weight == 0)
            spt.distance = 0;
        else if (weight > 0)
            spt.distance = weight;
        else
            spt.distance = -1;
        sptSet.push_back(spt);
    }
    return sptSet;
}

void Network::doLsrpAlgorithm()
{
    sortRoutersById();
    for (int i = 0; i < routers.size(); i++)
        routers[i]->deleteLsps();
    for (int i = 0; i < routers.size(); i++)
    {
        LSP lsp;
        lsp.source = routers[i];
        for (int j = 0; j < routers.size(); j++)
        {
            int weight = getLinkWeight(routers[i]->getId(), routers[j]->getId());
            if (weight > 0)
                lsp.adjacents.insert(pair<Router*, int>(routers[j], weight)); 
        }
        routers[i]->addLsp(lsp);
    }
    bool changed = true;
    while (changed)
    {
        changed = false;
        for (int i = 0; i < routers.size(); i++)
            for (int j = 0; j < routers.size(); j++)
            {
                int weightTemp = getLinkWeight(routers[i]->getId(), routers[j]->getId());
                if (weightTemp > 0)
                    if (routers[j]->exchangeLsps(routers[i]->getLsps())) 
                        changed = true;
            }
    }
}

Routes Network::getLsrpResults(int src)
{

}

void Network::doDvrpAlgorithm()
{
    sortRoutersById();
    for (int i = 0; i < routers.size(); i++)
        routers[i]->deleteRoutes();
    for (int i = 0; i < routers.size(); i++)
    {
        for (int j = 0; j < routers.size(); j++)
        {
            Route route;
            route.destination = routers[j];
            route.nextHop = NULL;
            route.path.push_back(routers[i]);
            route.cost = INT8_MAX;
            int weight = getLinkWeight(routers[i]->getId(), routers[j]->getId());
            if (weight == 0)
            {
                route.nextHop = routers[i];
                route.cost = 0;
            }
            else if (weight > 0)
            {
                route.nextHop = routers[j];
                route.cost = weight;
                route.path.push_back(routers[j]);
            }
            routers[i]->addRoute(route);
        }
    }
    bool changed = true;
    while (changed)
    {
        changed = false;
        for (int i = 0; i < routers.size(); i++)
            for (int j = 0; j < routers.size(); j++)
            {
                int weightTemp = getLinkWeight(routers[i]->getId(), routers[j]->getId());
                if (weightTemp > 0)
                    if (routers[j]->exchangeRoutes(routers[i], weightTemp, routers[i]->getRoutes())) 
                        changed = true;
            }
    }
}

Routes Network::getDvrpResults(int src)
{
    for (int i = 0; i < routers.size(); i++)
        if (routers[i]->getId() == src)
            return routers[i]->getRoutes();
}

int Network::getLinkWeight(int routerId1, int routerId2)
{
    for (int i = 0; i < links.size(); i++)
    {
        if ((links[i]->getFirstRouter()->getId() == routerId1 && links[i]->getSecondRouter()->getId() == routerId2)
            || (links[i]->getFirstRouter()->getId() == routerId2 && links[i]->getSecondRouter()->getId() == routerId1))
        {
            return links[i]->getWeight();
        }
    }
    if (routerId1 == routerId2)
        return 0;
    return -1;
}

vector<int> Network::getRoutersIds()
{
    vector<int> routersIds;
    sortRoutersById();
    for (int i = 0; i < routers.size(); i++)
        routersIds.push_back(routers[i]->getId());
    return routersIds;
}


void Network::sortRoutersById()
{
    sort(routers.begin(), routers.end(), [](Router* lhs, Router* rhs) 
            {
                return lhs->getId() < rhs->getId();
            });
}

Router* Network::getRouter(int routerId)
{
    for (int i = 0; i < routers.size(); i++)
        if (routers[i]->getId() == routerId)
            return routers[i];
    Router* newRouter = new Router(routerId);
    routers.push_back(newRouter);
    return newRouter;
}

void Network::addLink(Router* r1, Router* r2, int weight)
{
    Link* newLink = new Link(r1, r2, weight);
    links.push_back(newLink);
}

///////////////////////////////////////////////// INTERFACE /////////////////////////////////////////////////

class Interface
{
public:

    Interface(Network* n) {network = n; dvrpDone = false; lsrpDone = false;}
    void getOrder();

private:

    void addTopology();
    void showTopology();
    void doLsrpAlgorithm();
    vector<vector<int> > makeGraph();
    void doDvrpAlgorithm();
    void printDvrpResults(int src);
    void printLsrpResults(int src);
    void printLsrpItrResulat(SptSet sptSet);
    void modifyLink();
    void removeLink();

    bool dvrpDone;
    bool lsrpDone;
    Network* network;
};

void Interface::printDvrpResults(int src)
{
    cout << endl << endl;
    Routes routes = network->getDvrpResults(src);
    cout << "Dest\tNext Hop\tDist\tShortest Path" << endl;
    cout << "--------------------------------------------------" << endl;
    for (int i = 0; i < routes.size(); i++)
    {
        cout << routes[i].destination->getId() << "\t" << routes[i].nextHop->getId();
        cout << "\t\t" << routes[i].cost << "\t[";
        for (int j = 0; j < routes[i].path.size() - 1; j++)
            cout << routes[i].path[j]->getId() << "->";
        cout << routes[i].path[routes[i].path.size() - 1]->getId() << "]" << endl;
    }
    cout << endl;
}

void Interface::doDvrpAlgorithm()
{
    int src;
    string read;
    getline(cin, read);
    stringstream ss;
    ss << read << " 0";
    ss >> src;
    if (!dvrpDone)
    {
        network->doDvrpAlgorithm();
        dvrpDone = true;
    }
    vector<int> routers = network->getRoutersIds();
    if (src == 0)
        for (int i = 0; i < routers.size(); i++)
            printDvrpResults(routers[i]);
    else    
        printDvrpResults(src);
}

vector<vector<int> > Interface::makeGraph()
{
    vector<int> routersIds = network->getRoutersIds();
    vector<int> netGraphTemp(routersIds.size(), 0);
    vector<vector<int> > networkGraph(routersIds.size(), netGraphTemp);
    for (int i = 0; i < routersIds.size(); i++)
        for (int j = 0; j < routersIds.size(); j++)
            networkGraph[i][j] = network->getLinkWeight(routersIds[i], routersIds[j]);
    return networkGraph;
}

void printLsrpItrResulat(SptSet sptSet)
{

}

void Interface::printLsrpResults(int src)
{
    SptSet sptSet = network->getSptSet(src);
    SptSet newSptSet;
    Router* source;
    for (int i = 0; i < sptSet.size(); i++)
        if (sptSet[i].destination->getId() == src)
            source = sptSet[i].destination;
    vector<vector<Router*> > pathes;
    for (int i = 0; i < sptSet.size(); i++)
    {
        vector<Router*> path = {source, sptSet[i].destination};
        pathes.push_back(path);
    }
    LSPs lsps = source->getLsps();
    for (int i = 0; i < sptSet.size(); i++)
    {
        if (sptSet[i].destination->getId() == source->getId())
        {
            newSptSet.push_back(sptSet[i]);
            sptSet.erase(sptSet.begin() + i);
        }
    }
    int ittr = 1;
    while(sptSet.size() > 0)
    {
        cout << "\nIter" << ittr << ":\n" << endl;
        cout << "Dest\t|\t";
        for (int i = 0; i < newSptSet.size(); i++)
            cout << newSptSet[i].destination->getId() << "\t|\t";
        for (int i = 0; i < sptSet.size(); i++)
            cout << sptSet[i].destination->getId() << "\t|\t";
        cout << endl << "Cost\t|\t";
        for (int i = 0; i < newSptSet.size(); i++)
            cout << newSptSet[i].distance << "\t|\t";
        for (int i = 0; i < sptSet.size(); i++)
            cout << sptSet[i].distance << "\t|\t";
        cout << endl;
        for (int i = 0; i < sptSet.size() + newSptSet.size(); i++)
            cout << "------------";
        cout << endl;
        ittr++;

        int minI = 0;
        for (int i = 0; i < sptSet.size(); i++)
            if (sptSet[i].distance != -1)
            {
                minI = i;
                break;
            }
        for (int i = 0; i < sptSet.size(); i++)
            if (sptSet[i].distance != -1 && sptSet[i].distance < sptSet[minI].distance)
                minI = i;
        newSptSet.push_back(sptSet[minI]);
        sptSet.erase(sptSet.begin() + minI);
        map<Router*, int> adjacents;
        for (int i = 0; i < lsps.size(); i++)
            if (lsps[i].source->getId() == newSptSet[newSptSet.size() - 1].destination->getId())
                adjacents = lsps[i].adjacents;
        map<Router*, int>::iterator itr;
        for (itr = adjacents.begin(); itr != adjacents.end(); ++itr) 
        {
            int foundI;
            for (int i = 0; i < sptSet.size(); i++)
            {
                if (sptSet[i].destination->getId() == itr->first->getId())
                    if (sptSet[i].distance == -1 || newSptSet[newSptSet.size() - 1].distance + 
                        network->getLinkWeight(newSptSet[newSptSet.size() - 1].destination->getId(), itr->first->getId()) < 
                        sptSet[i].distance)
                    {
                        sptSet[i].distance = newSptSet[newSptSet.size() - 1].distance + 
                        network->getLinkWeight(newSptSet[newSptSet.size() - 1].destination->getId(), itr->first->getId());
                        for (int j = 0; j < pathes.size(); j++)
                        {
                            if (pathes[j][pathes[j].size() - 1]->getId() == sptSet[i].destination->getId())
                            {
                                for (int k = 0; k < pathes.size(); k++)
                                {
                                    if (pathes[k][pathes[k].size() - 1]->getId() == newSptSet[newSptSet.size() - 1].destination->getId())
                                    {
                                        pathes[j] = pathes[k];
                                        pathes[j].push_back(sptSet[i].destination);
                                    }
                                }
                            }
                        }
                    }  
            }
        }
    }
    cout << "\nPath:[s]->[d]\tMin-Cost\tShortest Path" << endl;
    cout << "----------------------------------------------" << endl;
    for (int i = 0; i < pathes.size(); i++)
    {
        if (pathes[i][pathes[i].size() - 1]->getId() == source->getId())
            continue;
        cout << "[" << pathes[i][0]->getId() << "]->[" << pathes[i][pathes[i].size() - 1]->getId() << "]\t";
        for (int j = 0; j < newSptSet.size(); j++)
            if (newSptSet[j].destination->getId() == pathes[i][pathes[i].size() - 1]->getId())
                cout << newSptSet[j].distance << "\t\t";
        for (int j = 0; j < pathes[i].size() - 1; j++)
            cout << pathes[i][j]->getId() << "->";
        cout << pathes[i][pathes[i].size() - 1]->getId() << endl;
    }
    cout << endl;
}

void Interface::doLsrpAlgorithm()
{
    int src;
    string read;
    getline(cin, read);
    stringstream ss;
    ss << read << " 0";
    ss >> src;
    if (!lsrpDone)
    {
        network->doLsrpAlgorithm();
        lsrpDone = true;
    }
    vector<int> routers = network->getRoutersIds();
    if (src == 0)
        for (int i = 0; i < routers.size(); i++)
            printLsrpResults(routers[i]);
    else    
        printLsrpResults(src);
}

void Interface::showTopology()
{
    vector<int> routersIds = network->getRoutersIds();
    cout << "\tu|v\t|\t";
    for (int i = 0; i < routersIds.size(); i++)
        cout << routersIds[i] << "\t";
    cout << endl;
    for (int i = 0; i < routersIds.size(); i++)
        cout << "-----------";
    for (int i = 0; i < routersIds.size(); i++)
    {
        cout << endl << "\t" << routersIds[i] << "\t|\t";
        for (int j = 0; j < routersIds.size(); j++)
            cout << network->getLinkWeight(routersIds[i], routersIds[j]) << "\t";
    }
    cout << endl;
}

void Interface::modifyLink()
{
    int s, d, c;
    char dash;
    cin >> s >> dash >> d >> dash >> c;
    network->modifyLink(s, d, c);
    if (s == d)
    {
        cout << "Source and destination cant be the same!!" << endl;
        return;
    }
    lsrpDone = false;
    dvrpDone = false;
}

void Interface::removeLink()
{
    int s, d;
    char dash;
    cin >> s >> dash >> d;
    network->removeLink(s, d);
    if (s == d)
    {
        cout << "Source and destination cant be the same!!" << endl;
        return;
    }
    lsrpDone = false;
    dvrpDone = false;
}

void Interface::addTopology()
{
    string relation;
    getline(cin, relation);
    stringstream ss;
    ss << relation << ' ';
    getline(ss, relation, ' ');
    while (getline(ss, relation, ' '))
    {
        stringstream ss2;
        ss2 << relation;
        int s, d, c;
        char dash;
        ss2 >> s >> dash >> d >> dash >> c;
        if (s == d)
        {
            cout << "Source and destination cant be the same!!" << endl;
            continue;
        }
        network->addLink(network->getRouter(s), network->getRouter(d), c);
    }
}

void Interface::getOrder()
{
    while (true)
    {
        string order;
        cin >> order;
        if (order == "topology")
            addTopology();
        else if (order == "show")
            showTopology();
        else if (order == "lsrp")
            doLsrpAlgorithm();
        else if (order == "dvrp")
            doDvrpAlgorithm();
        else if (order == "modify")
            modifyLink();
        else if (order == "remove")
            removeLink();   
    }
}

///////////////////////////////////////////////// MAIN /////////////////////////////////////////////////

int main()
{
    Network* network = new Network();
    Interface* interface = new Interface(network);
    interface->getOrder();
}