#include <iostream>
#include <map>
#include <queue>
#include <utility>
#include <climits>
#include <functional>
#include <iomanip>
#include <stack>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <tuple>


using namespace std;

#define SHORTEST_DISTANCE 0
#define PREVIOUS_NODE 1
#define VISITED 2

#define SET(func_name,type,var_name,_var_name) void func_name(type _var_name) { var_name = _var_name ;} 
#define GET(func_name,type,var_name) type func_name() const { return var_name ;}

class header;
class payload;
class packet;
class node;
class event;
class link; // new

// for simplicity, we use a const int to simulate the delay
// if you want to simulate the more details, you should revise it to be a class
const unsigned int ONE_HOP_DELAY = 10;
const unsigned int BROCAST_ID = UINT_MAX;

// BROCAST_ID means that all neighbors are receivers; UINT_MAX is the maximum value of unsigned int

class header {
    public:
        virtual ~header() {}

        SET(setSrcID, unsigned int , srcID, _srcID);
        SET(setDstID, unsigned int , dstID, _dstID);
        SET(setPreID, unsigned int , preID, _preID);
        SET(setNexID, unsigned int , nexID, _nexID);
        GET(getSrcID, unsigned int , srcID);
        GET(getDstID, unsigned int , dstID);
        GET(getPreID, unsigned int , preID);
        GET(getNexID, unsigned int , nexID);
        
        virtual string type() = 0;
        
        // factory concept: generate a header
        class header_generator {
                // lock the copy constructor
                header_generator(header_generator &){}
                // store all possible types of header
                static map<string,header_generator*> prototypes;
            protected:
                // allow derived class to use it
                header_generator() {}
                // after you create a new header type, please register the factory of this header type by this function
                void register_header_type(header_generator *h) { prototypes[h->type()] = h; }
                // you have to implement your own generate() to generate your header
                virtual header* generate() = 0 ;
            public:
                // you have to implement your own type() to return your header type
        	    virtual string type() = 0 ;
        	    // this function is used to generate any type of header derived
        	    static header * generate (string type) {
            		if(prototypes.find(type) != prototypes.end()){ // if this type derived exists 
            			return prototypes[type]->generate(); // generate it!!
            		}
            		std::cerr << "no such header type" << std::endl; // otherwise
            		return nullptr;
            	}
            	static void print () {
            	    cout << "registered header types: " << endl;
            	    for (map<string,header::header_generator*>::iterator it = prototypes.begin(); it != prototypes.end(); it ++)
            	        cout << it->second->type() << endl;
            	}
            	virtual ~header_generator(){};
        };
        
    protected:
        header():srcID(BROCAST_ID),dstID(BROCAST_ID),preID(BROCAST_ID),nexID(BROCAST_ID){} // this constructor cannot be directly called by users

    private:
        unsigned int srcID;
        unsigned int dstID;
        unsigned int preID;
        unsigned int nexID;
        header(header&){} // this constructor cannot be directly called by users
};
map<string,header::header_generator*> header::header_generator::prototypes;

class TRA_data_header : public header{
        TRA_data_header(TRA_data_header&s): labels(s.labels) {} // cannot be called by users
        
        stack<unsigned int> labels; // you can use the stack to store labels
        
    protected:
        TRA_data_header(){} // this constructor cannot be directly called by users

    public:
        ~TRA_data_header(){}
        string type() { return "TRA_data_header"; }
        
        void push_label(unsigned int _id) { labels.push(_id); }
        void pop_label() { labels.pop(); }
        unsigned int get_num_labels() { return labels.size(); } // although the original returned value's type size_t, we change it for simplicity
        unsigned int get_label() { return labels.size() ? labels.top() : 0; }

        class TRA_data_header_generator;
        friend class TRA_data_header_generator;
        // TRA_data_header_generator is derived from header_generator to generate a header
        class TRA_data_header_generator : public header_generator{
                static TRA_data_header_generator sample;
                // this constructor is only for sample to register this header type
                TRA_data_header_generator() { /*cout << "TRA_data_header registered" << endl;*/ register_header_type(&sample); }
            protected:
                virtual header * generate(){
                    // cout << "TRA_data_header generated" << endl;
                    return new TRA_data_header; 
                }
            public:
                virtual string type() { return "TRA_data_header";}
                ~TRA_data_header_generator(){}
        
        };
};
TRA_data_header::TRA_data_header_generator TRA_data_header::TRA_data_header_generator::sample;

class TRA_ctrl_header : public header{
        TRA_ctrl_header(TRA_ctrl_header&){} // cannot be called by users
        
    protected:
        TRA_ctrl_header(){} // this constructor cannot be directly called by users

    public:
        ~TRA_ctrl_header(){}
        string type() { return "TRA_ctrl_header"; }

        class TRA_ctrl_header_generator;
        friend class TRA_ctrl_header_generator;
        // TRA_ctrl_header_generator is derived from header_generator to generate a header
        class TRA_ctrl_header_generator : public header_generator{
                static TRA_ctrl_header_generator sample;
                // this constructor is only for sample to register this header type
                TRA_ctrl_header_generator() { /*cout << "TRA_ctrl_header registered" << endl;*/ register_header_type(&sample); }
            protected:
                virtual header * generate(){
                    // cout << "TRA_ctrl_header generated" << endl;
                    return new TRA_ctrl_header; 
                }
            public:
                virtual string type() { return "TRA_ctrl_header";}
                ~TRA_ctrl_header_generator(){}
        
        };
};
TRA_ctrl_header::TRA_ctrl_header_generator TRA_ctrl_header::TRA_ctrl_header_generator::sample;

class payload {
        payload(payload&){} // this constructor cannot be directly called by users
        
        string msg;
        
    protected:
        payload(){}
    public:
        virtual ~payload(){}
        virtual string type() = 0;
        
        SET(setMsg,string,msg,_msg);
        GET(getMsg,string,msg);
        
        class payload_generator {
                // lock the copy constructor
                payload_generator(payload_generator &){}
                // store all possible types of header
                static map<string,payload_generator*> prototypes;
            protected:
                // allow derived class to use it
                payload_generator() {}
                // after you create a new payload type, please register the factory of this payload type by this function
                void register_payload_type(payload_generator *h) { prototypes[h->type()] = h; }
                // you have to implement your own generate() to generate your payload
                virtual payload* generate() = 0;
            public:
                // you have to implement your own type() to return your header type
        	    virtual string type() = 0;
        	    // this function is used to generate any type of header derived
        	    static payload * generate (string type) {
            		if(prototypes.find(type) != prototypes.end()){ // if this type derived exists 
            			return prototypes[type]->generate(); // generate it!!
            		}
            		std::cerr << "no such payload type" << std::endl; // otherwise
            		return nullptr;
            	}
            	static void print () {
            	    cout << "registered payload types: " << endl;
            	    for (map<string,payload::payload_generator*>::iterator it = prototypes.begin(); it != prototypes.end(); it ++)
            	        cout << it->second->type() << endl;
            	}
            	virtual ~payload_generator(){};
        };
};
map<string,payload::payload_generator*> payload::payload_generator::prototypes;

class TRA_data_payload : public payload {
        TRA_data_payload(TRA_data_payload&){}

    protected:
        TRA_data_payload(){} // this constructor cannot be directly called by users
    public:
        ~TRA_data_payload(){}
        
        string type() { return "TRA_data_payload"; }
        
        class TRA_data_payload_generator;
        friend class TRA_data_payload_generator;
        // TRA_data_payload is derived from payload_generator to generate a payload
        class TRA_data_payload_generator : public payload_generator{
                static TRA_data_payload_generator sample;
                // this constructor is only for sample to register this payload type
                TRA_data_payload_generator() { /*cout << "TRA_data_payload registered" << endl;*/ register_payload_type(&sample); }
            protected:
                virtual payload * generate(){ 
                    // cout << "TRA_data_payload generated" << endl;
                    return new TRA_data_payload; 
                }
            public:
                virtual string type() { return "TRA_data_payload";}
                ~TRA_data_payload_generator(){}
        };
};
TRA_data_payload::TRA_data_payload_generator TRA_data_payload::TRA_data_payload_generator::sample;

class TRA_ctrl_payload : public payload {
        TRA_ctrl_payload(TRA_ctrl_payload & s): n_id(s.n_id), netw_info(s.netw_info){} //counter (s.counter) {}
        
        // unsigned int counter = 0;
        vector<unsigned int> checkSentPacket;

        unsigned n_id = 0; // default is zero
        map<unsigned int, pair<double,double> > netw_info;
        
    protected:
        TRA_ctrl_payload() {}  //: counter (0) {} // this constructor cannot be directly called by users
    public:
        ~TRA_ctrl_payload(){}
        
        // void increase() { counter ++; } // used to increase the counter
        // GET(getCounter,unsigned int,counter); // used to get the value of counter
        SET(setnid,unsigned int,n_id,_n_id);
        GET(getnid,unsigned int,n_id);
        void addNetwInfo(unsigned int nb_id, double link_capacity, double occupied) {
            //  if (netw_info.find(nb_id) == netw_info.end())
            netw_info[nb_id] = make_pair(link_capacity, occupied);
        }
        bool getSentPacket(unsigned int node){
            if (find(checkSentPacket.begin(), checkSentPacket.end(), node) != checkSentPacket.end()) 
                return true;
            return false;
        }
        void printVector(unsigned int node){
            cout << "packetID = " << node << endl;
            cout << "Current checkSentPacket contents: ";
            for (auto n : checkSentPacket) std::cout << n << " ";
            cout << "\n";
        }
        void setSentPacket(unsigned int node){
            if (find(checkSentPacket.begin(), checkSentPacket.end(), node) == checkSentPacket.end()) {
                checkSentPacket.push_back(node);
            }
        }

        map<unsigned int, pair<double, double> > getNetwInfo() {
            return netw_info;
        }
        
        string type() { return "TRA_ctrl_payload"; }
        
        class TRA_ctrl_payload_generator;
        friend class TRA_ctrl_payload_generator;
        // TRA_data_payload is derived from payload_generator to generate a payload
        class TRA_ctrl_payload_generator : public payload_generator{
                static TRA_ctrl_payload_generator sample;
                // this constructor is only for sample to register this payload type
                TRA_ctrl_payload_generator() { /*cout << "TRA_ctrl_payload registered" << endl;*/ register_payload_type(&sample); }
            protected:
                virtual payload * generate(){ 
                    // cout << "TRA_ctrl_payload generated" << endl;
                    return new TRA_ctrl_payload; 
                }
            public:
                virtual string type() { return "TRA_ctrl_payload";}
                ~TRA_ctrl_payload_generator(){}
        };
};
TRA_ctrl_payload::TRA_ctrl_payload_generator TRA_ctrl_payload::TRA_ctrl_payload_generator::sample;

class packet{
        // a packet usually contains a header and a payload
        header *hdr;
        payload *pld;
        unsigned int p_id;
        static unsigned int last_packet_id ;
        
        packet(packet &) {}
        static int live_packet_num ;
    protected:
        double size = 0; // packet size (flow size) is set to 0 by default (especially for control msg)
    
        // these constructors cannot be directly called by users
        packet(): hdr(nullptr), pld(nullptr) { p_id=last_packet_id++; live_packet_num ++; }
        packet(string _hdr, string _pld, double _size = 0, bool rep = false, unsigned int rep_id = 0) {
            if (! rep ) // a duplicated packet does not have a new packet id
                p_id = last_packet_id ++;
            else
                p_id = rep_id;
            hdr = header::header_generator::generate(_hdr); 
            pld = payload::payload_generator::generate(_pld);
            size = _size;
            live_packet_num ++;
        }
    public:
        virtual ~packet(){ 
            // cout << "packet destructor begin" << endl;
            if (hdr != nullptr) 
                delete hdr; 
            if (pld != nullptr) 
                delete pld; 
            live_packet_num --;
            // cout << "packet destructor end" << endl;
        }
        
        SET(setHeader,header*,hdr,_hdr);
        GET(getHeader,header*,hdr);
        SET(setPayload,payload*,pld,_pld);
        GET(getPayload,payload*,pld);
        GET(getPacketID,unsigned int,p_id);
        
        SET(setSize,unsigned int,size,_size);
        GET(getSize,unsigned int,size);
        
        static void discard ( packet* &p ) {
            // cout << "checking" << endl;
            if (p != nullptr) {
                // cout << "discarding" << endl;
                // cout << p->type() << endl;
                delete p;
                // cout << "discarded" << endl;
            }
            p = nullptr;
            // cout << "checked" << endl;
        }
        virtual string type () = 0;
        // you can define your own packet's addition_information
        // to print more information for recv_event and send_event
        virtual string addition_information () { return ""; }
        
        static int getLivePacketNum () { return live_packet_num; }
        
        class packet_generator;
        friend class packet_generator;
        class packet_generator {
                // lock the copy constructor
                packet_generator(packet_generator &){}
                // store all possible types of packet
                static map<string,packet_generator*> prototypes;
            protected:
                // allow derived class to use it
                packet_generator() {}
                // after you create a new packet type, please register the factory of this payload type by this function
                void register_packet_type(packet_generator *h) { prototypes[h->type()] = h; }
                // you have to implement your own generate() to generate your payload
                virtual packet* generate ( packet *p = nullptr) = 0;
            public:
                // you have to implement your own type() to return your packet type
        	    virtual string type() = 0;
        	    // this function is used to generate any type of packet derived
        	    static packet * generate (string type) {
            		if(prototypes.find(type) != prototypes.end()){ // if this type derived exists 
            			return prototypes[type]->generate(); // generate it!!
            		}
            		std::cerr << "no such packet type" << std::endl; // otherwise
            		return nullptr;
            	}
            	static packet * replicate (packet *p) {
            	    if(prototypes.find(p->type()) != prototypes.end()){ // if this type derived exists 
            			return prototypes[p->type()]->generate(p); // generate it!!
            		}
            		std::cerr << "no such packet type" << std::endl; // otherwise
            		return nullptr;
            	}
            	static void print () {
            	    cout << "registered packet types: " << endl;
            	    for (map<string,packet::packet_generator*>::iterator it = prototypes.begin(); it != prototypes.end(); it ++)
            	        cout << it->second->type() << endl;
            	}
            	virtual ~packet_generator(){};
        };
};
map<string,packet::packet_generator*> packet::packet_generator::prototypes;
unsigned int packet::last_packet_id = 0 ;
int packet::live_packet_num = 0;

// this packet is used to tell the destination the msg
class TRA_data_packet: public packet {
        TRA_data_packet(TRA_data_packet &) {}
        
    protected:
        TRA_data_packet(){} // this constructor cannot be directly called by users
        TRA_data_packet(packet*p): packet(p->getHeader()->type(), p->getPayload()->type(), p->getSize(), true, p->getPacketID()) {
            *(dynamic_cast<TRA_data_header*>(this->getHeader())) = *(dynamic_cast<TRA_data_header*> (p->getHeader()));
            *(dynamic_cast<TRA_data_payload*>(this->getPayload())) = *(dynamic_cast<TRA_data_payload*> (p->getPayload()));
            //DFS_path = (dynamic_cast<TRA_data_header*>(p))->DFS_path;
            //isVisited = (dynamic_cast<TRA_data_header*>(p))->isVisited;
        } // for duplicate
        TRA_data_packet(string _h, string _p, double _size = 0): packet(_h,_p,_size) {}
        
    public:
        virtual ~TRA_data_packet(){}
        string type() { return "TRA_data_packet"; }
        virtual string addition_information() {
            double size = getSize();
            // cout << counter << endl;
            TRA_data_header * _hdr = dynamic_cast<TRA_data_header*>(getHeader());
            unsigned int label = 0;
            if ( _hdr && _hdr->get_num_labels() )
                return " label " + to_string(_hdr->get_label()) ; // " size " + to_string(size);
            return " label x";
        }
        
        class TRA_data_packet_generator;
        friend class TRA_data_packet_generator;
        // TRA_data_packet is derived from packet_generator to generate a pub packet
        class TRA_data_packet_generator : public packet_generator{
                static TRA_data_packet_generator sample;
                // this constructor is only for sample to register this packet type
                TRA_data_packet_generator() { /*cout << "TRA_data_packet registered" << endl;*/ register_packet_type(&sample); }
            protected:
                virtual packet *generate (packet *p = nullptr){
                    // cout << "TRA_data_packet generated" << endl;
                    if ( nullptr == p )
                        return new TRA_data_packet("TRA_data_header","TRA_data_payload");  // generate a new packet
                    else
                        return new TRA_data_packet(p); // duplicate
                }
            public:
                virtual string type() { return "TRA_data_packet";}
                ~TRA_data_packet_generator(){}
        };
};
TRA_data_packet::TRA_data_packet_generator TRA_data_packet::TRA_data_packet_generator::sample;

// this packet type is used to tell the node what should be installed in the routing table
class TRA_ctrl_packet: public packet {
        TRA_ctrl_packet(TRA_ctrl_packet &) {}
        
    protected:
        TRA_ctrl_packet(){} // this constructor cannot be directly called by users
        TRA_ctrl_packet(packet*p): packet(p->getHeader()->type(), p->getPayload()->type(), p->getSize(), true, p->getPacketID()) {
            *(dynamic_cast<TRA_ctrl_header*>(this->getHeader())) = *(dynamic_cast<TRA_ctrl_header*> (p->getHeader()));
            *(dynamic_cast<TRA_ctrl_payload*>(this->getPayload())) = *(dynamic_cast<TRA_ctrl_payload*> (p->getPayload()));
            //DFS_path = (dynamic_cast<TRA_ctrl_header*>(p))->DFS_path;
            //isVisited = (dynamic_cast<TRA_ctrl_header*>(p))->isVisited;
        } // for duplicate
        TRA_ctrl_packet(string _h, string _p, double _size = 0): packet(_h,_p,_size) {}
        
    public:
        virtual ~TRA_ctrl_packet(){}
        string type() { return "TRA_ctrl_packet"; }
        virtual string addition_information() {
            // unsigned int counter = (dynamic_cast<TRA_ctrl_payload*>(this->getPayload()))->getCounter();
            unsigned int id = (dynamic_cast<TRA_ctrl_payload*>(this->getPayload()))->getnid();
            // cout << counter << endl;
            return " from " + to_string(id);
        }
        
        class TRA_ctrl_packet_generator;
        friend class TRA_ctrl_packet_generator;
        // TRA_ctrl_packet is derived from packet_generator to generate a pub packet
        class TRA_ctrl_packet_generator : public packet_generator{
                static TRA_ctrl_packet_generator sample;
                // this constructor is only for sample to register this packet type
                TRA_ctrl_packet_generator() { /*cout << "TRA_ctrl_packet registered" << endl;*/ register_packet_type(&sample); }
            protected:
                virtual packet *generate (packet *p = nullptr){
                    // cout << "TRA_ctrl_packet generated" << endl;
                    if ( nullptr == p )
                        return new TRA_ctrl_packet("TRA_ctrl_header","TRA_ctrl_payload"); // generate a new packet
                    else
                        return new TRA_ctrl_packet(p); // duplicate
                }
            public:
                virtual string type() { return "TRA_ctrl_packet";}
                ~TRA_ctrl_packet_generator(){}
        };
};
TRA_ctrl_packet::TRA_ctrl_packet_generator TRA_ctrl_packet::TRA_ctrl_packet_generator::sample;

class node {
        // all nodes created in the program
        static map<unsigned int, node*> id_node_table;
        map<unsigned int,bool> phy_neighbors;
        unsigned int id;
        unsigned int num_of_label;
    protected:
        node(node&){} // this constructor should not be used
        node(){} // this constructor should not be used
        node(unsigned int _id): id(_id) { id_node_table[_id] = this; }
    public:
        virtual ~node() { // erase the node
            id_node_table.erase (id) ;
        }
        virtual string type() = 0; // please define it in your derived node class
        
        void add_phy_neighbor (unsigned int _id, string link_type = "simple_link", map<string, double> link_args = {} ); // we only add a directed link from id to _id
        void del_phy_neighbor (unsigned int _id); // we only delete a directed link from id to _id
        
        // you can use the function to get the node's neigbhors at this time
        const map<unsigned int,bool> & getPhyNeighbors () { 
            return phy_neighbors;
        }
        GET(getNumOfLabel, unsigned int, num_of_label);
        SET(setNumOfLabel, unsigned int, num_of_label, _num_of_label);
        
        link * getLink(unsigned int nb_id);
        
        void recv (packet *p) {
            packet *tp = p;
            recv_handler(tp); 
            packet::discard(p); 
        } // the packet will be directly deleted after the handler
        void send (packet *p);
        
        // receive the packet and do something; this is a pure virtual function
        virtual void recv_handler(packet *p) = 0;
        void send_handler(packet *P);
        
        static node * id_to_node (unsigned int _id) { return ((id_node_table.find(_id)!=id_node_table.end()) ? id_node_table[_id]: nullptr) ; }
        GET(getNodeID,unsigned int,id);
        
        static void del_node (unsigned int _id) {
            if (id_node_table.find(_id) != id_node_table.end())
                id_node_table.erase(_id);
        }
        static unsigned int getNodeNum () { return id_node_table.size(); }

        class node_generator {
                // lock the copy constructor
                node_generator(node_generator &){}
                // store all possible types of node
                static map<string,node_generator*> prototypes;
            protected:
                // allow derived class to use it
                node_generator() {}
                // after you create a new node type, please register the factory of this node type by this function
                void register_node_type(node_generator *h) { prototypes[h->type()] = h; }
                // you have to implement your own generate() to generate your node
                virtual node* generate(unsigned int _id, unsigned int switches, unsigned int links) = 0;
                // virtual node* generate(unsigned int _id) = 0;
            public:
                // you have to implement your own type() to return your node type
        	    virtual string type() = 0;
        	    // this function is used to generate any type of node derived
        	    static node * generate (string type, unsigned int _id, unsigned int total_switches, unsigned int total_links) {
        	        if(id_node_table.find(_id)!=id_node_table.end()){
        	            std::cerr << "duplicate node id" << std::endl; // node id is duplicated
        	            return nullptr;
        	        }
        	        else if ( BROCAST_ID == _id ) {
        	            cerr << "BROCAST_ID cannot be used" << endl;
        	            return nullptr;
        	        }
            		else if(prototypes.find(type) != prototypes.end()){ // if this type derived exists 
            		    node * created_node = prototypes[type]->generate(_id, total_switches, total_links);
            			return created_node; // generate it!!
            		}
            		std::cerr << "no such node type" << std::endl; // otherwise
            		return nullptr;
            	}
            	static void print () {
            	    cout << "registered node types: " << endl;
            	    for (map<string,node::node_generator*>::iterator it = prototypes.begin(); it != prototypes.end(); it ++)
            	        cout << it->second->type() << endl;
            	}
            	virtual ~node_generator(){};
        };
};
map<string,node::node_generator*> node::node_generator::prototypes;
map<unsigned int,node*> node::id_node_table;

// my TRA_swi
class TRA_switch: public node {
        bool hi;
        vector<unsigned int> checkReceived;
        vector<unsigned int> labelAdded;
        unordered_map<int, unordered_map<unsigned int, double>> visitedLabel;

        unordered_set<unsigned int> receivedPacketIDs; // Track whether received before
        map<unsigned int, unsigned int> routingTable; // Destination, next node ID

        map<unsigned int, map<unsigned int, pair<double, double>>> networkTopology; // From, To, capacity, occupied
        map<unsigned int, map<unsigned int, unsigned int>> mapInitiate; // source, destination, next node ID
        
        unordered_map<unsigned int, double> instanceNI;

        // my declaration
        void setHi(unsigned int packetID);
        bool getHi(unsigned int packetID);
        void storeLabel(int t, unsigned int label, double occupied);
        int canUse(int t, unsigned int label);
        
        void setMap(unsigned int source, unsigned int destination, unsigned int nextNode);
        unsigned int getMap(unsigned int source, unsigned int destination);
        void setNI(unsigned int n_id, double available);
        double getNI(unsigned int n_id);

        bool matching(unsigned int currentNode, unsigned int previousNode, int nodeTable[][3]);
        void traverseNetwork(
            unsigned int source, unsigned int destination, 
            const map<unsigned int, map<unsigned int, pair<double, double>>>& topology,
            int nodeTable[][3], bool buildLabels, vector<unsigned int>* path,
            map<unsigned int, vector<pair<unsigned int, int>>>* labelTable);

        void nodeTableInitiate(unsigned int nodeCount, int nodeTable[][3]);
        int canTransmitFlow(const vector<unsigned int>& path, int flowSize,
            const map<unsigned int, map<unsigned int, pair<double, double>>>& networkTopology);

        void buildSegmentRoutingLabels(
            unsigned int currentNode, unsigned int nextNode, const vector<unsigned int>* path, 
            int nodeTable[][3], map<unsigned int, vector<pair<unsigned int, int>>>* labelTable);
        
        void shortestPaths(bool isCtrl, unsigned int source, unsigned int destination, 
            const map<unsigned int, map<unsigned int, pair<double, double>>>& networkTopology, 
            int nodeTable[][3]);

        int pop(vector<unsigned int>& stack, int nodeTable[][3]);
        void routingTableInitiate(unsigned int destination, unsigned int source, int nodeTable[][3]);
        void setRoutingTable(unsigned int destination, unsigned int nextNode);

        unsigned int total_switches;
        unsigned int total_links;

    protected:
        TRA_switch() {} // it should not be used
        TRA_switch(TRA_switch&) {} // it should not be used
        // TRA_switch(unsigned int _id): node(_id), hi(false) {} // this constructor cannot be directly called by users

    public:
        TRA_switch(unsigned int _id, unsigned int switches, unsigned int links): 
            node(_id), hi(false), total_switches(switches), total_links(links) {}
        ~TRA_switch(){}
        string type() { return "TRA_switch"; }
        
        // please define recv_handler function to deal with the incoming packet
        virtual void recv_handler (packet *p);
        
        double getCapacity(unsigned int nb_id);
        double getOccupied(unsigned int nb_id);
        map<unsigned int, unsigned int> getRoutingTable() { return routingTable; }
        unsigned int get_one_hop_neighbour(unsigned int destination);
        
        // void add_one_hop_neighbor (unsigned int n_id) { one_hop_neighbors[n_id] = true; }
        // unsigned int get_one_hop_neighbor_num () { return one_hop_neighbors.size(); }
        
        class TRA_switch_generator;
        friend class TRA_switch_generator;
        // TRA_switch is derived from node_generator to generate a node
        class TRA_switch_generator : public node_generator{
                static TRA_switch_generator sample;
                // this constructor is only for sample to register this node type
                TRA_switch_generator() { /*cout << "TRA_switch registered" << endl;*/ register_node_type(&sample); }
            protected:
                virtual node * generate(unsigned int _id, unsigned int switches, unsigned int links) {
                    return new TRA_switch(_id, switches, links); }
                // virtual node * generate(unsigned int _id){ /*cout << "TRA_switch generated" << endl;*/ return new TRA_switch(_id); }
            public:
                virtual string type() { return "TRA_switch";}
                ~TRA_switch_generator(){}
        };
};
TRA_switch::TRA_switch_generator TRA_switch::TRA_switch_generator::sample;

class mycomp {
    bool reverse;
    
    public:
        mycomp(const bool& revparam = false) { reverse=revparam ; }
        bool operator() (const event* lhs, const event* rhs) const;
};

class event {
        event(event*&){} // this constructor cannot be directly called by users
        static priority_queue < event*, vector < event* >, mycomp > events;
        static unsigned int cur_time; // timer
        static unsigned int end_time;
        
        // get the next event
        static event * get_next_event() ;
        static void add_event (event *e) { events.push(e); }
        static hash<string> event_seq;
        
    protected:
        unsigned int trigger_time;
        
        event(){} // it should not be used
        event(unsigned int _trigger_time): trigger_time(_trigger_time) {}

    public:
        virtual void trigger()=0;
        virtual ~event(){}

        virtual unsigned int event_priority() const = 0;
        unsigned int get_hash_value(string string_for_hash) const {
            unsigned int priority = event_seq (string_for_hash);
            return priority;
        }
        
        static void flush_events (); // only for debug
        
        GET(getTriggerTime,unsigned int,trigger_time);
        
        static void start_simulate( unsigned int _end_time ); // the function is used to start the simulation
        
        static unsigned int getCurTime() { return cur_time ; }
        static void getCurTime(unsigned int _cur_time) { cur_time = _cur_time; } 
        // static unsigned int getEndTime() { return end_time ; }
        // static void getEndTime(unsigned int _end_time) { end_time = _end_time; }
        
        virtual void print () const = 0; // the function is used to print the event information

        class event_generator{
                // lock the copy constructor
                event_generator(event_generator &){}
                // store all possible types of event
                static map<string,event_generator*> prototypes;
            protected:
                // allow derived class to use it
                event_generator() {}
                // after you create a new event type, please register the factory of this event type by this function
                void register_event_type(event_generator *h) { prototypes[h->type()] = h; }
                // you have to implement your own generate() to generate your event
                virtual event* generate(unsigned int _trigger_time, void * data) = 0;
            public:
                // you have to implement your own type() to return your event type
        	    virtual string type() = 0;
        	    // this function is used to generate any type of event derived
        	    static event * generate (string type, unsigned int _trigger_time, void * data) {
            		if(prototypes.find(type) != prototypes.end()){ // if this type derived exists
            		    event * e = prototypes[type]->generate(_trigger_time, data);
            		    add_event(e);
            		    return e; // generate it!!
            		}
            		std::cerr << "no such event type" << std::endl; // otherwise
            		return nullptr;
            	}
            	static void print () {
            	    cout << "registered event types: " << endl;
            	    for (map<string,event::event_generator*>::iterator it = prototypes.begin(); it != prototypes.end(); it ++)
            	        cout << it->second->type() << endl;
            	}
            	virtual ~event_generator(){}
        };
};
map<string,event::event_generator*> event::event_generator::prototypes;
priority_queue < event*, vector< event* >, mycomp > event::events;
hash<string> event::event_seq;

unsigned int event::cur_time = 0;
unsigned int event::end_time = 0;

void event::flush_events()
{ 
    cout << "**flush begin" << endl;
    while ( ! events.empty() ) {
        cout << setw(11) << events.top()->trigger_time << ": " << setw(11) << events.top()->event_priority() << endl;
        delete events.top();
        events.pop();
    }
    cout << "**flush end" << endl;
}
event * event::get_next_event() {
    if(events.empty()) 
        return nullptr; 
    event * e = events.top();
    events.pop(); 
    // cout << events.size() << " events remains" << endl;
    return e; 
}
void event::start_simulate(unsigned int _end_time) {
    if (_end_time<0) {
        cerr << "you should give a possitive value of _end_time" << endl;
        return;
    }
    end_time = _end_time;
    event *e; 
    e = event::get_next_event ();
    while ( e != nullptr && e->trigger_time <= end_time ) {
        if ( cur_time <= e->trigger_time )
            cur_time = e->trigger_time;
        else {
            cerr << "cur_time = " << cur_time << ", event trigger_time = " << e->trigger_time << endl;
            break;
        }

        // cout << "event trigger_time = " << e->trigger_time << endl;
        e->print(); // for log
        // cout << " event begin" << endl;
        e->trigger();
        // cout << " event end" << endl;
        delete e;
        e = event::get_next_event ();
    }
    // cout << "no more event" << endl;
}

bool mycomp::operator() (const event* lhs, const event* rhs) const {
    // cout << lhs->getTriggerTime() << ", " << rhs->getTriggerTime() << endl;
    // cout << lhs->type() << ", " << rhs->type() << endl;
    unsigned int lhs_pri = lhs->event_priority();
    unsigned int rhs_pri = rhs->event_priority();
    // cout << "lhs hash = " << lhs_pri << endl;
    // cout << "rhs hash = " << rhs_pri << endl;
    
    if (reverse) 
        return ((lhs->getTriggerTime()) == (rhs->getTriggerTime())) ? (lhs_pri < rhs_pri): ((lhs->getTriggerTime()) < (rhs->getTriggerTime()));
    else 
        return ((lhs->getTriggerTime()) == (rhs->getTriggerTime())) ? (lhs_pri > rhs_pri): ((lhs->getTriggerTime()) > (rhs->getTriggerTime()));
}

class recv_event: public event {
    public:
        class recv_data; // forward declaration
            
    private:
        recv_event(recv_event&) {} // this constructor cannot be used
        recv_event() {} // we don't allow users to new a recv_event by themselv
        unsigned int senderID; // the sender
        unsigned int receiverID; // the receiver; the packet will be given to the receiver
        packet *pkt; // the packet
        
    protected:
        // this constructor cannot be directly called by users; only by generator
        recv_event(unsigned int _trigger_time, void *data): event(_trigger_time), senderID(BROCAST_ID), receiverID(BROCAST_ID), pkt(nullptr){
            recv_data * data_ptr = (recv_data*) data;
            senderID = data_ptr->s_id;
            receiverID = data_ptr->r_id; // the packet will be given to the receiver
            pkt = data_ptr->_pkt;
        } 
        
    public:
        virtual ~recv_event(){}
        // recv_event will trigger the recv function
        virtual void trigger();
        
        unsigned int event_priority() const;
        
        class recv_event_generator;
        friend class recv_event_generator;
        // recv_event is derived from event_generator to generate a event
        class recv_event_generator : public event_generator{
                static recv_event_generator sample;
                // this constructor is only for sample to register this event type
                recv_event_generator() { /*cout << "recv_event registered" << endl;*/ register_event_type(&sample); }
            protected:
                virtual event * generate(unsigned int _trigger_time, void *data){ 
                    // cout << "recv_event generated" << endl; 
                    return new recv_event(_trigger_time, data); 
                }
                
            public:
                virtual string type() { return "recv_event";}
                ~recv_event_generator(){}
        };
        // this class is used to initialize the recv_event
        class recv_data{
            public:
                unsigned int s_id;
                unsigned int r_id;
                packet *_pkt;
        };
        
        void print () const;
};
recv_event::recv_event_generator recv_event::recv_event_generator::sample;

void recv_event::trigger() {
    if (pkt == nullptr) {
        cerr << "recv_event error: no pkt!" << endl; 
        return ; 
    }
    else if (node::id_to_node(receiverID) == nullptr){
        cerr << "recv_event error: no node " << receiverID << "!" << endl;
        delete pkt; return ;
    }
    node::id_to_node(receiverID)->recv(pkt); 
}
unsigned int recv_event::event_priority() const {
    string string_for_hash;
    string_for_hash = to_string(getTriggerTime()) + to_string(senderID) + to_string (receiverID) + to_string (pkt->getPacketID());
    return get_hash_value(string_for_hash);
}
// the recv_event::print() function is used for log file
void recv_event::print () const {
/*
   // if (pkt->type() == "TRA_ctrl_packet") return;

    cout << "time "          << setw(5) << event::getCurTime() 
         << "   recID"       << setw(5) << receiverID 
         << "   pktID"       << setw(5) << pkt->getPacketID()
         << "   srcID"       << setw(5) << pkt->getHeader()->getSrcID() 
         << "   dstID"       << setw(5) << pkt->getHeader()->getDstID() 
         << "   preID"       << setw(5) << pkt->getHeader()->getPreID()
         << "   nexID"       << setw(5) << pkt->getHeader()->getNexID()
         << "   "            << pkt->type()
         << pkt->addition_information();
         cout << endl;
/* */
    cout << "time "          << setw(11) << event::getCurTime() 
         << "   recID"       << setw(11) << receiverID 
         << "   pktID"       << setw(11) << pkt->getPacketID()
         << "   srcID"       << setw(11) << pkt->getHeader()->getSrcID() 
         << "   dstID"       << setw(11) << pkt->getHeader()->getDstID() 
         << "   preID"       << setw(11) << pkt->getHeader()->getPreID()
         << "   nexID"       << setw(11) << pkt->getHeader()->getNexID()
         << "   "            << pkt->type()
         << pkt->addition_information();
         cout << endl;

}

class send_event: public event {
    public:
        class send_data; // forward declaration
            
    private:
        send_event (send_event &){}
        send_event (){} // we don't allow users to new a recv_event by themselves
        // this constructor cannot be directly called by users; only by generator
        unsigned int senderID; // the sender
        unsigned int receiverID; // the receiver 
        packet *pkt; // the packet
    
    protected:
        send_event (unsigned int _trigger_time, void *data): event(_trigger_time), senderID(BROCAST_ID), receiverID(BROCAST_ID), pkt(nullptr){
            send_data * data_ptr = (send_data*) data;
            senderID = data_ptr->s_id;
            receiverID = data_ptr->r_id;
            pkt = data_ptr->_pkt;
        } 
        
    public:
        virtual ~send_event(){}
        // send_event will trigger the send function
        virtual void trigger();
        
        unsigned int event_priority() const;
        
        class send_event_generator;
        friend class send_event_generator;
        // send_event is derived from event_generator to generate a event
        class send_event_generator : public event_generator{
                static send_event_generator sample;
                // this constructor is only for sample to register this event type
                send_event_generator() { /*cout << "send_event registered" << endl;*/ register_event_type(&sample); }
            protected:
                virtual event * generate(unsigned int _trigger_time, void *data){ 
                    // cout << "send_event generated" << endl; 
                    return new send_event(_trigger_time, data); 
                }
            
            public:
                virtual string type() { return "send_event";}
                ~send_event_generator(){}
        };
        // this class is used to initialize the send_event
        class send_data{
            public:
                unsigned int s_id;
                unsigned int r_id;
                packet *_pkt;
                unsigned int t;
        };
        
        void print () const;
};
send_event::send_event_generator send_event::send_event_generator::sample;

void send_event::trigger() {
    if (pkt == nullptr) {
        cerr << "send_event error: no pkt!" << endl; 
        return ; 
    }
    else if (node::id_to_node(senderID) == nullptr){
        cerr << "send_event error: no node " << senderID << "!" << endl;
        delete pkt; return ;
    }
    node::id_to_node(senderID)->send(pkt);
}
unsigned int send_event::event_priority() const {
    string string_for_hash;
    string_for_hash = to_string(getTriggerTime()) + to_string(senderID) + to_string (receiverID) + to_string (pkt->getPacketID());
    return get_hash_value(string_for_hash);
}
// the send_event::print() function is used for log file
void send_event::print () const {

/*    // if (pkt->type() == "TRA_ctrl_packet") return;

    cout << "time "          << setw(5) << event::getCurTime() 
         << "   senID"       << setw(5) << senderID 
         << "   pktID"       << setw(5) << pkt->getPacketID()
         << "   srcID"       << setw(5) << pkt->getHeader()->getSrcID() 
         << "   dstID"       << setw(5) << pkt->getHeader()->getDstID() 
         << "   preID"       << setw(5) << pkt->getHeader()->getPreID()
         << "   nexID"       << setw(5) << pkt->getHeader()->getNexID()
         << "   "            << pkt->type()
         << pkt->addition_information()
         << endl;
/**/
    cout << "time "          << setw(11) << event::getCurTime() 
         << "   senID"       << setw(11) << senderID 
         << "   pktID"       << setw(11) << pkt->getPacketID()
         << "   srcID"       << setw(11) << pkt->getHeader()->getSrcID() 
         << "   dstID"       << setw(11) << pkt->getHeader()->getDstID() 
         << "   preID"       << setw(11) << pkt->getHeader()->getPreID()
         << "   nexID"       << setw(11) << pkt->getHeader()->getNexID()
         << "   "            << pkt->type()
         << pkt->addition_information()
         << endl;

}

////////////////////////////////////////////////////////////////////////////////

class TRA_data_pkt_gen_event: public event {
    public:
        class gen_data; // forward declaration
            
    private:
        TRA_data_pkt_gen_event (TRA_data_pkt_gen_event &){}
        TRA_data_pkt_gen_event (){} // we don't allow users to new a recv_event by themselves
        // this constructor cannot be directly called by users; only by generator
        unsigned int src; // the src
        unsigned int dst; // the dst 
        double size;
        // packet *pkt; // the packet
        string msg;
    
    protected:
        TRA_data_pkt_gen_event (unsigned int _trigger_time, void *data): event(_trigger_time), src(BROCAST_ID), dst(BROCAST_ID){
            pkt_gen_data * data_ptr = (pkt_gen_data*) data;
            src = data_ptr->src_id;
            dst = data_ptr->dst_id;
            size = data_ptr->_size;
            // pkt = data_ptr->_pkt;
            msg = data_ptr->msg;
        } 
        
    public:
        virtual ~TRA_data_pkt_gen_event(){}
        // TRA_data_pkt_gen_event will trigger the packet gen function
        virtual void trigger();
        
        unsigned int event_priority() const;
        
        class TRA_data_pkt_gen_event_generator;
        friend class TRA_data_pkt_gen_event_generator;
        // TRA_data_pkt_gen_event_generator is derived from event_generator to generate an event
        class TRA_data_pkt_gen_event_generator : public event_generator{
                static TRA_data_pkt_gen_event_generator sample;
                // this constructor is only for sample to register this event type
                TRA_data_pkt_gen_event_generator() { /*cout << "send_event registered" << endl;*/ register_event_type(&sample); }
            protected:
                virtual event * generate(unsigned int _trigger_time, void *data){ 
                    // cout << "send_event generated" << endl; 
                    return new TRA_data_pkt_gen_event(_trigger_time, data); 
                }
            
            public:
                virtual string type() { return "TRA_data_pkt_gen_event";}
                ~TRA_data_pkt_gen_event_generator(){}
        };
        // this class is used to initialize the TRA_data_pkt_gen_event
        class pkt_gen_data{
            public:
                unsigned int src_id;
                unsigned int dst_id;
                double _size;
                string msg;
                // packet *_pkt;
        };
        
        void print () const;
};
TRA_data_pkt_gen_event::TRA_data_pkt_gen_event_generator TRA_data_pkt_gen_event::TRA_data_pkt_gen_event_generator::sample;

void TRA_data_pkt_gen_event::trigger() {
    if (node::id_to_node(src) == nullptr){
        cerr << "TRA_data_pkt_gen_event error: no node " << src << "!" << endl;
        return ;
    }
    else if ( dst != BROCAST_ID && node::id_to_node(dst) == nullptr ) {
        cerr << "TRA_data_pkt_gen_event error: no node " << dst << "!" << endl;
        return;
    }
    
    TRA_data_packet *pkt = dynamic_cast<TRA_data_packet*> ( packet::packet_generator::generate("TRA_data_packet") );
    if (pkt == nullptr) { 
        cerr << "packet type is incorrect" << endl; return; 
    }
    TRA_data_header *hdr = dynamic_cast<TRA_data_header*> ( pkt->getHeader() );
    TRA_data_payload *pld = dynamic_cast<TRA_data_payload*> ( pkt->getPayload() );
    
    if (hdr == nullptr) {
        cerr << "header type is incorrect" << endl; return ;
    }
    if (pld == nullptr) {
        cerr << "payload type is incorrect" << endl; return ;
    }

    hdr->setSrcID(src);
    hdr->setDstID(dst);
    hdr->setPreID(src); // this column is not important when the packet is first received by the src (i.e., just generated)
    hdr->setNexID(src); // this column is not important when the packet is first received by the src (i.e., just generated)

    pld->setMsg(msg);
    
    pkt->setSize(size);
    
    // cout << "**size = " << pkt->getSize() << "**" << endl ;
    
    recv_event::recv_data e_data;
    e_data.s_id = src;
    e_data.r_id = src; // to make the packet start from the src
    e_data._pkt = pkt;
    
    recv_event *e = dynamic_cast<recv_event*> ( event::event_generator::generate("recv_event", trigger_time, (void *)&e_data) );

}
unsigned int TRA_data_pkt_gen_event::event_priority() const {
    string string_for_hash;
    string_for_hash = to_string(getTriggerTime()) + to_string(src) + to_string (dst) ; //to_string (pkt->getPacketID());
    return get_hash_value(string_for_hash);
}
// the TRA_data_pkt_gen_event::print() function is used for log file
void TRA_data_pkt_gen_event::print () const {
/*
    cout << "time "          << setw(5) << event::getCurTime() 
         << "        "       << setw(5) << " "
         << "        "       << setw(5) << " "
         << "   srcID"       << setw(5) << src
         << "   dstID"       << setw(5) << dst
         << "        "       << setw(5) << " "
         << "        "       << setw(5) << " "
         << "   TRA_data_packet generating" // << setw(5) << size
         << endl;
/* */
    cout << "time "          << setw(11) << event::getCurTime() 
        << "        "       << setw(11) << " "
        << "        "       << setw(11) << " "
        << "   srcID"       << setw(11) << src
        << "   dstID"       << setw(11) << dst
        << "        "       << setw(11) << " "
        << "        "       << setw(11) << " "
        << "   TRA_data_packet generating" // << setw(5) << size
        << endl;

}

class TRA_ctrl_pkt_gen_event: public event {
    public:
        class gen_data; // forward declaration
            
    private:
        TRA_ctrl_pkt_gen_event (TRA_ctrl_pkt_gen_event &){}
        TRA_ctrl_pkt_gen_event (){} // we don't allow users to new a recv_event by themselves
        // this constructor cannot be directly called by users; only by generator
        unsigned int src; // the src
        unsigned int dst; // the dst 
        double size;
        // unsigned int mat;
        // unsigned int act;
        // packet *pkt; // the packet
        string msg;
        // double per; // percentage
    
    protected:
        TRA_ctrl_pkt_gen_event (unsigned int _trigger_time, void *data): event(_trigger_time), src(BROCAST_ID), dst(BROCAST_ID){
            pkt_gen_data * data_ptr = (pkt_gen_data*) data;
            src = data_ptr->src_id;
            dst = data_ptr->dst_id;
            size = data_ptr->_size;
            // pkt = data_ptr->_pkt;
            // mat = data_ptr->mat_id;
            // act = data_ptr->act_id;
            msg = data_ptr->msg;
            // per = data_ptr->per;
        } 
        
    public:
        virtual ~TRA_ctrl_pkt_gen_event(){}
        // TRA_ctrl_pkt_gen_event will trigger the packet gen function
        virtual void trigger();
        
        unsigned int event_priority() const;
        
        class TRA_ctrl_pkt_gen_event_generator;
        friend class TRA_ctrl_pkt_gen_event_generator;
        // TRA_ctrl_pkt_gen_event_generator is derived from event_generator to generate an event
        class TRA_ctrl_pkt_gen_event_generator : public event_generator{
                static TRA_ctrl_pkt_gen_event_generator sample;
                // this constructor is only for sample to register this event type
                TRA_ctrl_pkt_gen_event_generator() { /*cout << "send_event registered" << endl;*/ register_event_type(&sample); }
            protected:
                virtual event * generate(unsigned int _trigger_time, void *data){ 
                    // cout << "send_event generated" << endl; 
                    return new TRA_ctrl_pkt_gen_event(_trigger_time, data); 
                }
            
            public:
                virtual string type() { return "TRA_ctrl_pkt_gen_event";}
                ~TRA_ctrl_pkt_gen_event_generator(){}
        };
        // this class is used to initialize the TRA_ctrl_pkt_gen_event
        class pkt_gen_data{
            public:
                unsigned int src_id;
                unsigned int dst_id;
                double _size;
                // unsigned int mat_id; // the target of the rule
                // unsigned int act_id; // the next hop toward the target recorded in the rule
                string msg;
                // double per; // the percentage
                // packet *_pkt;
        };
        
        void print () const;
};
TRA_ctrl_pkt_gen_event::TRA_ctrl_pkt_gen_event_generator TRA_ctrl_pkt_gen_event::TRA_ctrl_pkt_gen_event_generator::sample;

void TRA_ctrl_pkt_gen_event::trigger() {
    
    TRA_ctrl_packet *pkt = dynamic_cast<TRA_ctrl_packet*> ( packet::packet_generator::generate("TRA_ctrl_packet") );
    if (pkt == nullptr) { 
        cerr << "packet type is incorrect" << endl; return; 
    }
    TRA_ctrl_header *hdr = dynamic_cast<TRA_ctrl_header*> ( pkt->getHeader() );
    TRA_ctrl_payload *pld = dynamic_cast<TRA_ctrl_payload*> ( pkt->getPayload() );
    
    if (hdr == nullptr) {
        cerr << "header type is incorrect" << endl; return ;
    }
    if (pld == nullptr) {
        cerr << "payload type is incorrect" << endl; return ;
    }

    hdr->setSrcID(src); 
    hdr->setDstID(dst);
    hdr->setPreID(src);
    // hdr->setNexID(dst); // in hw3, you should set NexID to src
    hdr->setNexID(src);
    
    // payload
    pld->setMsg(msg);
    pld->setnid(src); // add this line
    // pld->setMatID(mat);
    // pld->setActID(act);
    // pld->setPer(per);
    pkt->setSize(size);
    
    recv_event::recv_data e_data;
    e_data.s_id = src;
    // e_data.r_id = dst; // in hw3, you should set r_id it src
    e_data.r_id = src;
    e_data._pkt = pkt;
    
    recv_event *e = dynamic_cast<recv_event*> ( event::event_generator::generate("recv_event",trigger_time, (void *)&e_data) );
}
unsigned int TRA_ctrl_pkt_gen_event::event_priority() const {
    string string_for_hash;
    // string_for_hash = to_string(getTriggerTime()) + to_string(src) + to_string(dst) + to_string(mat) + to_string(act); //to_string (pkt->getPacketID());
    string_for_hash = to_string(getTriggerTime()) + to_string(src) + to_string(dst) ; //to_string (pkt->getPacketID());
    return get_hash_value(string_for_hash);
}
// the TRA_ctrl_pkt_gen_event::print() function is used for log file
void TRA_ctrl_pkt_gen_event::print () const {
/*
    cout << "time "          << setw(5) << event::getCurTime() 
         << "        "       << setw(5) << " "
         << "        "       << setw(5) << " "
         << "   srcID"       << setw(5) << src
         << "   dstID"       << setw(5) << dst
         << "        "       << setw(5) << " "
         << "        "       << setw(5) << " "
         << "   TRA_ctrl_packet generating"
         << endl;
/**/
    cout << "time "          << setw(11) << event::getCurTime() 
         << "        "       << setw(11) << " "
         << "        "       << setw(11) << " "
         << "   srcID"       << setw(11) << src
         << "   dstID"       << setw(11) << dst
         << "        "       << setw(11) << " "
         << "        "       << setw(11) << " "
         << "   TRA_ctrl_packet generating"
         << endl;

}

////////////////////////////////////////////////////////////////////////////////

class link {
        // all links created in the program
        static map< pair<unsigned int,unsigned int>, link*> id_id_link_table;
        
        unsigned int id1; // from
        unsigned int id2; // to
        
    protected:
        link(link&){} // this constructor should not be used
        link(){} // this constructor should not be used
        link(unsigned int _id1, unsigned int _id2): id1(_id1), id2(_id2) { id_id_link_table[pair<unsigned int,unsigned int>(id1,id2)] = this; }

    public:
        virtual ~link() { 
            id_id_link_table.erase (pair<unsigned int,unsigned int>(id1,id2)); // erase the link
        }
        
        static link * id_id_to_link (unsigned int _id1, unsigned int _id2) { 
            return ((id_id_link_table.find(pair<unsigned int,unsigned int>(_id1,_id2))!=id_id_link_table.end()) ? id_id_link_table[pair<unsigned,unsigned>(_id1,_id2)]: nullptr) ; 
        }

        virtual unsigned int getLatency() = 0; // you must implement your own latency
        
        static void del_link (unsigned int _id1, unsigned int _id2) {
            pair<unsigned int, unsigned int> temp;
            if (id_id_link_table.find(temp)!=id_id_link_table.end())
                id_id_link_table.erase(temp); 
        }

        static unsigned int getLinkNum () { return id_id_link_table.size(); }
        
        virtual bool canTransmit ( map<string,double> args={} ) = 0;
        virtual void reserve( map<string,double> args={} ) = 0;

        class link_generator {
                // lock the copy constructor
                link_generator(link_generator &){}
                // store all possible types of link
                static map<string,link_generator*> prototypes;
            protected:
                // allow derived class to use it
                link_generator() {}
                // after you create a new link type, please register the factory of this link type by this function
                void register_link_type(link_generator *h) { prototypes[h->type()] = h; }
                // you have to implement your own generate() to generate your link
                virtual link* generate(unsigned int _id1, unsigned int _id2, map<string, double> args = {}) = 0;
            public:
                // you have to implement your own type() to return your link type
        	    virtual string type() = 0;
        	    // this function is used to generate any type of link derived
        	    static link * generate (string type, unsigned int _id1, unsigned int _id2, map<string, double> args = {}) {
        	        if(id_id_link_table.find(pair<unsigned int,unsigned int>(_id1,_id2))!=id_id_link_table.end()){
        	            std::cerr << "duplicate link id" << std::endl; // link id is duplicated
        	            return nullptr;
        	        }
        	        else if ( BROCAST_ID == _id1 || BROCAST_ID == _id2 ) {
        	            cerr << "BROCAST_ID cannot be used" << endl;
        	            return nullptr;
        	        }
            		else if (prototypes.find(type) != prototypes.end()){ // if this type derived exists 
            		    link * created_link = prototypes[type]->generate(_id1, _id2, args);
            			return created_link; // generate it!!
            		}
            		std::cerr << "no such link type" << std::endl; // otherwise
            		return nullptr;
            	}
            	static void print () {
            	    cout << "registered link types: " << endl;
            	    for (map<string,link::link_generator*>::iterator it = prototypes.begin(); it != prototypes.end(); it ++)
            	        cout << it->second->type() << endl;
            	}
            	virtual ~link_generator(){};
        };
};
map<string,link::link_generator*> link::link_generator::prototypes;
map<pair<unsigned int,unsigned int>, link*> link::id_id_link_table;

void node::add_phy_neighbor (unsigned int _id, string link_type, map<string, double> link_args){
    if (id == _id) return; // if the two nodes are the same...
    if (id_node_table.find(_id)==id_node_table.end()) return; // if this node does not exist
    if (phy_neighbors.find(_id)!=phy_neighbors.end()) return; // if this neighbor has been added
    phy_neighbors[_id] = true;
    
    link::link_generator::generate(link_type, id,_id, link_args);
}
void node::del_phy_neighbor (unsigned int _id){
    phy_neighbors.erase(_id);
    
}

class simple_link: public link {
        double capacity = 100; // default capacity is set to 100; please set your link's capacity manually by add_phy_neighbor
        double occupied = 0; // default occupied is set to 0
        
        unsigned int latency = ONE_HOP_DELAY; // default latency
        
    protected:
        simple_link() {} // it should not be used outside the class
        simple_link(simple_link&) {} // it should not be used
        simple_link(unsigned int _id1, unsigned int _id2): link (_id1,_id2){} // this constructor cannot be directly called by users
    
    public:
        virtual ~simple_link() {}
        virtual unsigned int getLatency() { return latency; } // you can implement your own latency
        
        SET(setCapacity,unsigned int,capacity,_capacity);
        GET(getCapacity,double,capacity);
        GET(getOccupied,double,occupied);
        
        SET(setLatency,unsigned int,latency,_latency);
        
        bool canTransmit(map<string, double> args = {}) {
            if (args.find("pkt_size") != args.end())
                return ( occupied + args["pkt_size"] <= capacity );
            return false;
        }
        void reserve(map<string, double> args = {}) {
            if (args.find("pkt_size") != args.end())
                occupied += args["pkt_size"];
        }
        
        class simple_link_generator;
        friend class simple_link_generator;
        // simple_link is derived from link_generator to generate a link
        class simple_link_generator : public link_generator {
                static simple_link_generator sample;
                // this constructor is only for sample to register this link type
                simple_link_generator() { /*cout << "simple_link registered" << endl;*/ register_link_type(&sample); }
            protected:
                virtual link * generate(unsigned int _id1, unsigned int _id2, map<string, double> args = {}) 
                { /*cout << "simple_link generated" << endl;*/ 
                    simple_link * l = new simple_link(_id1,_id2);
                    if (args.find("capacity") != args.end())
                        l->setCapacity(static_cast<unsigned int> (args["capacity"]));
                    if (args.find("latency") != args.end())
                        l->setLatency(static_cast<unsigned int> (args["latency"]));
                    return l;
                }
            public:
                virtual string type() { return "simple_link"; }
                ~simple_link_generator(){}
        };
};

simple_link::simple_link_generator simple_link::simple_link_generator::sample;


// the data_packet_event function is used to add an initial event
void data_packet_event (unsigned int src, unsigned int dst, double f_size, unsigned int t = 0, string msg = "default"){
    if ( node::id_to_node(src) == nullptr || (dst != BROCAST_ID && node::id_to_node(dst) == nullptr) ) {
        cerr << "src or dst is incorrect" << endl; return ;
        return;
    }

    TRA_data_pkt_gen_event::pkt_gen_data e_data;
    e_data.src_id = src;
    e_data.dst_id = dst;
    e_data._size = f_size;
    e_data.msg = msg;
    
    // recv_event *e = dynamic_cast<recv_event*> ( event::event_generator::generate("recv_event",t, (void *)&e_data) );
    TRA_data_pkt_gen_event *e = dynamic_cast<TRA_data_pkt_gen_event*> ( event::event_generator::generate("TRA_data_pkt_gen_event",t, (void *)&e_data) );
    if (e == nullptr) cerr << "event type is incorrect" << endl;
}

// the TRA_ctrl_packet_event function is used to add an initial event
void TRA_ctrl_packet_event (unsigned int src, unsigned int t = event::getCurTime(),
                        string msg = "default") {
        // 1st parameter: the source; the destination that want to broadcast a msg with counter 0 (i.e., match ID)
        // 2nd parameter: time (optional)
        // 3rd parameter: msg for debug information (optional)
    if ( node::id_to_node(src) == nullptr ) {
        cerr << "id is incorrect" << endl; return;
    }
    
    // unsigned int src = con_id;
    TRA_ctrl_pkt_gen_event::pkt_gen_data e_data;
    e_data.src_id = src;
    e_data.dst_id = BROCAST_ID;
    // e_data.mat_id = mat;
    // e_data.act_id = act;
    e_data._size = 0; // we assume ctrl msg size is zero 
    e_data.msg = msg;
    // e_data.per = per;
    
    TRA_ctrl_pkt_gen_event *e = dynamic_cast<TRA_ctrl_pkt_gen_event*> ( event::event_generator::generate("TRA_ctrl_pkt_gen_event",t, (void *)&e_data) );
    if (e == nullptr) cerr << "event type is incorrect" << endl;
}

link * node::getLink(unsigned int nb_id) {
    return link::id_id_to_link (getNodeID(), nb_id);
}

// send_handler function is used to transmit packet p based on the information in the header
// Note that the packet p will not be discard after send_handler ()
void node::send_handler(packet *p){
    packet *_p = packet::packet_generator::replicate(p);
    send_event::send_data e_data;
    e_data.s_id = _p->getHeader()->getPreID();
    e_data.r_id = _p->getHeader()->getNexID();
    e_data._pkt = _p;
    send_event *e = dynamic_cast<send_event*> (event::event_generator::generate("send_event",event::getCurTime(), (void *)&e_data) );
    if (e == nullptr) cerr << "event type is incorrect" << endl;
}

void node::send(packet *p){ // this function is called by event; not for the user; you cannot change the send function
    if (p == nullptr) return;
    
    unsigned int _nexID = p->getHeader()->getNexID();
    for ( map<unsigned int,bool>::iterator it = phy_neighbors.begin(); it != phy_neighbors.end(); it ++) {
        unsigned int nb_id = it->first; // neighbor id
        
        if (nb_id != _nexID && BROCAST_ID != _nexID) continue; // this neighbor will not receive the packet
        
        link *l = link::id_id_to_link(id, nb_id);
        // if ( !l ) continue; // no such a link
        
        double pkt_size = p->getSize();
        if ( ! (l->canTransmit( {{"pkt_size", pkt_size}} )) ) continue; // if the capacity is insufficient, drop the packet
        l->reserve( {{"pkt_size", pkt_size}} );
        
        unsigned int trigger_time = event::getCurTime() + link::id_id_to_link(id, nb_id)->getLatency() ; // we simply assume that the delay is fixed
        // cout << "node " << id << " send to node " <<  nb_id << endl;
        recv_event::recv_data e_data;
        e_data.s_id = id;    // set the sender   (i.e., preID)
        e_data.r_id = nb_id; // set the receiver (i.e., nexID)
        
        packet *p2 = packet::packet_generator::replicate(p);
        e_data._pkt = p2;
        
        recv_event *e = dynamic_cast<recv_event*> (event::event_generator::generate("recv_event", trigger_time, (void*) &e_data)); // send the packet to the neighbor
        if (e == nullptr) cerr << "event type is incorrect" << endl;
    }
    packet::discard(p);
}

double TRA_switch::getCapacity(unsigned int nb_id){
    simple_link *l = dynamic_cast<simple_link*>(getLink(nb_id));
    if (l) return l->getCapacity();
    return 0;
}
double TRA_switch::getOccupied(unsigned int nb_id){
    simple_link *l = dynamic_cast<simple_link*>(getLink(nb_id));
    if (l) return l->getOccupied();
    return 0;
}
unsigned int TRA_switch::get_one_hop_neighbour(unsigned int label) {
    if (routingTable.find(label) != routingTable.end()) return routingTable.at(label);
    return -1;
}
void TRA_switch::setRoutingTable(unsigned int destination, unsigned int nextNode){
    routingTable[destination] = nextNode;
}
void TRA_switch::setHi(unsigned int packetID){
    if (find(checkReceived.begin(), checkReceived.end(), packetID) == checkReceived.end()) {
        checkReceived.push_back(packetID);
    }
}
bool TRA_switch::getHi(unsigned int packetID){
    if (find(checkReceived.begin(), checkReceived.end(), packetID) != checkReceived.end()) {
        return true;
    }
    return false;
}
void TRA_switch::setMap(unsigned int source, unsigned int destination, unsigned int nextNode) {
    mapInitiate[source][destination] = nextNode;
}
unsigned int TRA_switch::getMap(unsigned int source, unsigned int destination) {
    if (mapInitiate.find(source) == mapInitiate.end()) return -1;
    if (mapInitiate[source].find(destination) == mapInitiate[source].end()) return -1;
    return mapInitiate[source][destination];
}
void TRA_switch::setNI(unsigned int n_id, double available) {
    auto it = instanceNI.find(n_id);
    if (it == instanceNI.end()) instanceNI[n_id] = available;
    else if (available < it->second) it->second = available;
}
double TRA_switch::getNI(unsigned int n_id) {
    auto it = instanceNI.find(n_id);
    if (it != instanceNI.end()) return it->second;
    return -1.0; 
}
int TRA_switch::canTransmitFlow(
    const vector<unsigned int>& path, int flowSize,
    const map<unsigned int, map<unsigned int, pair<double, double>>>& networkTopology) {
    int minimumAvailable = INT_MAX;
    for (size_t i = 0; i < path.size() - 1; i++) {
        unsigned int from = path[i];
        unsigned int to = path[i+1];
        
        auto from_node_it = networkTopology.find(from);
        if (from_node_it == networkTopology.end()) return 0;
        
        auto to_link_it = from_node_it->second.find(to);
        if (to_link_it == from_node_it->second.end()) return 0;
        
        const auto& link = to_link_it->second;
        int available = link.first - link.second;
        if (available < flowSize) {
            // cout << "cap = " << link.first << ", occ = " << link.second << endl;
            return 0;
        }
        minimumAvailable = (minimumAvailable > available) ? available : minimumAvailable; 
    }
    return minimumAvailable;
}
// my function
void TRA_switch::storeLabel(int t, unsigned int label, double occupied) {
    visitedLabel[t][label] += occupied;
}
int TRA_switch::canUse(int t, unsigned int label) {
    auto time_it = visitedLabel.find(t);
    if (time_it == visitedLabel.end()) return 0;
    
    auto label_it = time_it->second.find(label);
    if (label_it == time_it->second.end()) return 0;

    return visitedLabel[t][label];
}

bool TRA_switch::matching(unsigned int currentNode, unsigned int previousNode, int nodeTable[][3]) {
    vector<unsigned int> path1;
    unsigned int current = currentNode;
    while (current != -1) {
        path1.push_back(current);
        current = nodeTable[current][PREVIOUS_NODE];
    }
    reverse(path1.begin(), path1.end());

    vector<unsigned int> path2;
    current = previousNode;
    while (current != -1) {
        path2.push_back(current);
        current = nodeTable[current][PREVIOUS_NODE];
    }
    reverse(path2.begin(), path2.end());

    for (int i = 0; i < min(path1.size(), path2.size()); i++) {
        if (path1[i] != path2[i]) {
            return (path1[i] < path2[i]); // If current is better will return true
        }
    }
    return false;
}
int TRA_switch::pop(vector<unsigned int>& stack, int nodeTable[][3]) 
{
    int index = 0;
    for (long unsigned int i = 1; i < stack.size(); i++) {
        if (nodeTable[stack[i]][SHORTEST_DISTANCE] < nodeTable[stack[index]][SHORTEST_DISTANCE]) index = i;
    }

    int distance = nodeTable[stack[index]][SHORTEST_DISTANCE];
    for (long unsigned int i = 1; i < stack.size(); i++) {
        if (i == index) continue;

        if (nodeTable[stack[i]][SHORTEST_DISTANCE] == distance) {
            if (matching(stack[i], stack[index], nodeTable)) {
                index = i;
                break;
            }    
        }
    }
    int popNode = stack[index];
    stack.erase(stack.begin() + index);
    return popNode;
}
void TRA_switch::buildSegmentRoutingLabels(
    // my build segment
    unsigned int currentNode,
    unsigned int nextNode,
    const vector<unsigned int>* path,
    int nodeTable[][3],
    map<unsigned int, vector<pair<unsigned int, int>>>* labelTable) {
    
    // Create subpath from nextNode back to source
    vector<unsigned int> subpath;
    unsigned int current = currentNode;
    while (current != -1) {
        if (current == (*path)[path->size() - 1]) return;
        subpath.push_back(current);
        current = nodeTable[current][PREVIOUS_NODE];
    }
    reverse(subpath.begin(), subpath.end());

    // Check for matches between path prefixes and subpath nodes
    int match1 = -1;
    int match2 = -1;
    for (int i = 0; i < path->size(); i++) {
        if (nextNode == (*path)[i]) {
            for (int loop1 = 0; loop1 < i; loop1++) {
                for (int loop2 = 0; loop2 < subpath.size(); loop2++) {
                    if ((*path)[loop1] == subpath[loop2]) {
                        match1 = loop1;
                        match2 = loop2;
                        // cout << "MATCH: " << (*path)[loop1] << " = " << subpath[loop2] << endl;
                    }
                }
            }
        }
    }
    if (match1 == -1 || match2 == subpath.size() - 1 || match1 == path->size() - 1) return;

    for(int i = 0; i < path->size(); i++)
    {
        if ((*path)[match1] != subpath[match2]) {
            // cout << "difference at here: " << (*path)[match1] << ", " << subpath[match2] << endl;
            (*labelTable)[(*path)[match1]].emplace_back(subpath[match2], subpath.size());
            break;
        }
        match1++;
        match2++;
    }
    match1 = path->size() -1;
    match2 = subpath.size() -1;
    for(int i = path->size() - 1; i > 0; i--)
    {
        if ((*path)[match1] != subpath[match2]) {
            (*labelTable)[(*path)[match1]].emplace_back(subpath[match2], subpath.size());
            break;
        }
        match1--;
        match2--;
    }
}


// Implement this code in recv_handler of TRA_switch. Each node processes received packets here.
void TRA_switch::recv_handler (packet *p){
    if (p == nullptr) return;
    
    // my recv ctrl
    if (p->type() == "TRA_ctrl_packet") { // the switch receives a packet from the controller
        TRA_ctrl_packet *p3 = nullptr;
        p3 = dynamic_cast<TRA_ctrl_packet*> (p);
        TRA_ctrl_payload *l3 = nullptr;
        l3 = dynamic_cast<TRA_ctrl_payload*> (p3->getPayload());
        
        // Return if received before
        if(getHi(p3->getPacketID())) return;
        setHi(p3->getPacketID());

        // Set packet data
        p3->getHeader()->setPreID ( getNodeID() );
        p3->getHeader()->setNexID ( BROCAST_ID );
        p3->getHeader()->setDstID ( BROCAST_ID );

        unsigned int currentNode = getNodeID();
        unsigned int source = p3->getHeader()->getSrcID();

        if( source == currentNode ) // this packet is sent from this node
        {
            l3->setnid( currentNode );  // use this line to set the node ID
            map<unsigned int,bool> nblist = getPhyNeighbors();

            for ( map<unsigned int,bool>::iterator it = nblist.begin(); it != nblist.end(); it ++) 
            {
                unsigned nb_id = it->first; // nb id
                double capacity = getCapacity(nb_id);
                double occupied = getOccupied(nb_id);
                l3->addNetwInfo(nb_id, capacity, occupied);
                networkTopology[currentNode][nb_id] = {capacity, occupied};
                setNI(nb_id, capacity - occupied);
            }
        }
        else // this packet is sent from the other node
        {
            map<unsigned int, pair<double, double>> src_nbs = l3->getNetwInfo();

            for (const auto& [neighbour, cap_occ] : src_nbs) {
                auto& srcMap = networkTopology[source]; // Automatically creates srcID if it doesn't exist
                auto it = srcMap.find(neighbour);
                
                if (it == srcMap.end() || cap_occ.second > it->second.second) {
                    srcMap[neighbour] = cap_occ; // Insert or update if occupied is greater
                }
            }

            // networkTopology count
            unsigned int count = 0;
            for (const auto& node_entry : networkTopology) {
                for (const auto& link_entry : node_entry.second) count++;
            }

            if ((!hi) && (count == total_links)) {
                // cout << "   ******** do routin table :>" << endl;
                int nodeTable[total_switches][3];
                for (unsigned int source = 0; source < total_switches; source++) {
                    for (unsigned int destination = 0; destination < total_switches; destination++) {
                        if (source == destination || getMap(source, destination) != UINT_MAX) continue;
                        nodeTableInitiate(total_switches, nodeTable);
                        shortestPaths(1, source, destination, networkTopology, nodeTable);
                    }
                }

                if (mapInitiate.find(currentNode) != mapInitiate.end()) {
                    const auto& destinations = mapInitiate[currentNode];
                    for (const auto& entry : destinations) setRoutingTable(entry.first, entry.second);
                }
                hi = true;
            }  
        }
        send_handler(p3);
    }
    else if (p->type() == "TRA_data_packet") {
        TRA_data_packet *p4 = dynamic_cast<TRA_data_packet*>(p);
        TRA_data_header *h4 = dynamic_cast<TRA_data_header*>(p4->getHeader());
        TRA_data_payload *l4 = dynamic_cast<TRA_data_payload*>(p4->getPayload());

        if(getHi(p4->getPacketID())) return; // return if received before
        if(getRoutingTable().empty()) return; // return if no routingTable yet

        setHi(p4->getPacketID()); // Set this packet as visited
        h4->setPreID(getNodeID()); // Set current node as previous node

        unsigned int currentNode = getNodeID();
        unsigned int source = h4->getSrcID();
        unsigned int destination = h4->getDstID();
        int flowSize = p4->getSize();

        if (h4->getSrcID() == getNodeID()) { 
            // my recv data
            h4->push_label(destination);

            unsigned int time = event::getCurTime();
            int occupied = canUse(time, destination);

            // dp the shortest path of this pair
            int nodeTable[total_switches][3];
            nodeTableInitiate(total_switches, nodeTable);
            shortestPaths(0, source, destination, networkTopology, nodeTable);
            unsigned int current = destination;
            vector<unsigned int> path;
            while (current != -1) 
            {
                path.push_back(current);
                current = nodeTable[current][PREVIOUS_NODE];
            }
            reverse(path.begin(), path.end());

            int check1 = canTransmitFlow(path, occupied + flowSize, networkTopology);
            if (check1) storeLabel(time, destination, flowSize);
            else { // Cannot OSPF
                map<unsigned int, vector<pair<unsigned int, int>>> labelTable;
                nodeTableInitiate(total_switches, nodeTable);
                traverseNetwork(source, destination, networkTopology, nodeTable, true, &path, &labelTable);

                pair<int, unsigned int> best_path = {-1, 0}; // Shortest distance, label

                for (auto& entry : labelTable) {
                    sort(entry.second.begin(), entry.second.end(), 
                        [](const pair<unsigned int, int>& a, const pair<unsigned int, int>& b) {
                            return a.second < b.second;  // Sort by distance (second element)
                        });
                }
                
                unsigned int label;
                int prevAvailable = 0;
                int prevDistance = -1 ;
                bool check3 = false;
                for (const auto& entry : labelTable) { // Try 1 label
                    for (const auto& p : entry.second) {
                        unsigned int label = p.first;

                        vector<unsigned int> tempPath;
                        unsigned int node = source;
                        bool valid_path = true;
                        int hop_count = 0;
                        const int MAX_HOPS = total_switches; // Prevent infinite loops

                        while (node != label && hop_count < MAX_HOPS) {
                            unsigned int next_node = getMap(node, label);
                            if (next_node == node) {
                                valid_path = false;
                                break;
                            }
                            tempPath.push_back(node);
                            node = next_node;
                            hop_count++;
                        }
                        if (!valid_path || hop_count >= MAX_HOPS) continue;

                        while (node != destination && hop_count < MAX_HOPS) {
                            unsigned int next_node = getMap(node, destination);
                            if (next_node == node) { 
                                valid_path = false;
                                break;
                            }
                            tempPath.push_back(node);
                            node = next_node;
                            hop_count++;
                        }

                        if (!valid_path || hop_count >= MAX_HOPS) continue;

                        occupied = canUse(time, destination);
                        int check2 = canTransmitFlow(tempPath, occupied + p4->getSize(), networkTopology);
                        if(!check2) continue;

                        if (prevDistance == tempPath.size()) {
                            if (check2 > prevAvailable) {
                                best_path = {tempPath.size(), label};
                                prevAvailable = check2;
                                prevDistance = tempPath.size();
                            }
                        } 
                        else if (prevDistance == -1 || tempPath.size() < prevDistance) {
                            best_path = {tempPath.size(), label};
                            prevAvailable = check2;
                            prevDistance = tempPath.size();
                        }
                    }
                }

                if (best_path.first != -1) {
                    unsigned int currentNumLabel = h4->get_num_labels();
                    unsigned int maxNumLabel = getNumOfLabel();
                    if (currentNumLabel + 1 > maxNumLabel) return;
                    h4->push_label(best_path.second);
                    storeLabel(time, best_path.second, flowSize);
                }
                else { // Try more labels
                    vector<unsigned int> labels;
                    vector<unsigned int> tempPath;

                    unsigned int prevNode = source;
                    unsigned int node = prevNode;
                    unsigned int prevLabelDis = -1;

                    int hop_count = 0;
                    int prevCount = hop_count;

                    bool valid_path = true;
                    const int MAX_HOPS = total_switches; // Prevent infinite loops

                    for (const auto& entry : labelTable) {
                        for (const auto& p : entry.second) {
                            unsigned int label = p.first;          
                            while (node != label && hop_count < MAX_HOPS) {
                                unsigned int next_node = getMap(node, label);
                                if (next_node == node) {
                                    valid_path = false;
                                    break;
                                }
                                tempPath.push_back(node);
                                node = next_node;
                                hop_count++;
                            }
                            
                            if (!valid_path || hop_count >= MAX_HOPS) {
                                valid_path = true;
                                hop_count = prevCount;
                                tempPath.clear();
                                node = prevNode;
                                continue;
                            }
                            occupied = canUse(time, destination);
                            int check2 = canTransmitFlow(tempPath, occupied + p4->getSize(), networkTopology);
                            if(!check2) {
                                hop_count = prevCount;
                                tempPath.clear();
                                node = prevNode;
                                continue;
                            }
                            else {
                                unsigned int maxNumLabel = getNumOfLabel();
                                if (labels.size() + 1 > maxNumLabel) break;
                                else if (prevLabelDis != -1 && prevLabelDis == p.second) {
                                    if (check2 > prevAvailable) {
                                        labels.push_back(label);
                                        tempPath.clear();
                                        prevCount = hop_count;
                                        prevNode = node;
                                    }
                                }
                                else {
                                    labels.push_back(label);
                                    tempPath.clear();
                                    prevCount = hop_count;
                                    prevNode = node;
                                    prevAvailable = check2;
                                    prevLabelDis = p.second;
                                }
                            }
                        }
                    }

                    if (!labels.empty()) {
                        for (const auto& label : labels) {
                            unsigned int currentNumLabel = h4->get_num_labels();
                            unsigned int maxNumLabel = getNumOfLabel();
                            if (currentNumLabel + 1 > maxNumLabel) break;
                            h4->push_label(label);
                        }
                    }
                    else {
                        for(int node_i = 0; node_i < total_switches; node_i++) {
                            if (node_i == source) continue;
                            if (node_i == destination) continue;
                            for (const auto& entry : labelTable) 
                                for (const auto& p : entry.second) 
                                    if (node_i == p.first) continue;

                            pair<int, unsigned int> best_path = {-1, 0};

                            label = node_i;
                            vector<unsigned int> tempPath;
                            bool valid_path = true;
                            unsigned int node = source;
                            int hop_count = 0;
                            int prevCount = hop_count;
                            const int MAX_HOPS = total_switches;
                            unsigned int prevDistance = -1;
                            unsigned int prevAvailable = -1;

                            while (node != label && hop_count < MAX_HOPS) {
                                unsigned int next_node = getMap(node, label);
                                if (next_node == node) {
                                    valid_path = false;
                                    break;
                                }
                                tempPath.push_back(node);
                                node = next_node;
                                hop_count++;
                            }
                            if (!valid_path || hop_count >= MAX_HOPS) continue;

                            while (node != destination && hop_count < MAX_HOPS) {
                                unsigned int next_node = getMap(node, destination);
                                if (next_node == node) { 
                                    valid_path = false;
                                    break;
                                }
                                tempPath.push_back(node);
                                node = next_node;
                                hop_count++;
                            }
                            if (!valid_path || hop_count >= MAX_HOPS) continue;

                            occupied = canUse(time, destination);
                            int check4 = canTransmitFlow(tempPath, occupied + p4->getSize(), networkTopology);
                            if(!check4) continue;

                            if (prevDistance == tempPath.size()) {
                                if (check4 > prevAvailable) {
                                    best_path = {tempPath.size(), label};
                                    prevAvailable = check4;
                                    prevDistance = tempPath.size();
                                }
                            } 
                            else if (prevDistance == -1 || tempPath.size() < prevDistance) {
                                best_path = {tempPath.size(), label};
                                prevAvailable = check4;
                                prevDistance = tempPath.size();
                            }
                        }
                    
                        if (best_path.first != -1) {
                            unsigned int currentNumLabel = h4->get_num_labels();
                            unsigned int maxNumLabel = getNumOfLabel();
                            if (currentNumLabel + 1 > maxNumLabel) return;
                            h4->push_label(best_path.second);
                            storeLabel(time, best_path.second, flowSize);
                        }
                    }
                }
            }
        }
        else if (currentNode == destination) return; 
        else {
            // Try a better way to receive label
            unsigned int currentLabel = h4->get_label();
            // unsigned int numLabel = h4->get_num_labels();
            if (currentNode == currentLabel) h4->pop_label();
        }

        unsigned int nextID = get_one_hop_neighbour(h4->get_label());
        unsigned int available = getNI(nextID);
        if (available < p4->getSize()) return;

        h4->setNexID(nextID);
        setNI(nextID, available - p4->getSize());
        send_handler(p4);
    }
}

void TRA_switch:: nodeTableInitiate(unsigned int nodeCount, int nodeTable[][3])
{
    for (unsigned int i = 0; i < nodeCount; i++)
    {
        nodeTable[i][SHORTEST_DISTANCE] = INT_MAX;
        nodeTable[i][PREVIOUS_NODE] = -1;
        nodeTable[i][VISITED] = 0;
    }
}

void TRA_switch::traverseNetwork(
    // my traverse
    unsigned int source,
    unsigned int destination,
    const map<unsigned int, map<unsigned int, pair<double, double>>>& topology,
    int nodeTable[][3],
    bool buildLabels,
    vector<unsigned int>* path,
    map<unsigned int, vector<pair<unsigned int, int>>>* labelTable) { // Changed to unsigned int 


    vector<unsigned int> stack;  // Changed to unsigned int
    stack.push_back(source);
    nodeTable[source][SHORTEST_DISTANCE] = 0;
    
    if (buildLabels && labelTable) {
        (*labelTable)[source] = {};
    }

    while (!stack.empty()) {
        unsigned int currentNode = pop(stack, nodeTable);
        nodeTable[currentNode][VISITED] = 1;

        auto it = topology.find(currentNode);
        if (it != topology.end()) {
            for (const auto& link_entry : it->second) {
                unsigned int nextNode = link_entry.first;
                if (nodeTable[nextNode][VISITED] == 1) continue;

                int previousDistance = nodeTable[nextNode][SHORTEST_DISTANCE];
                int currentDistance = nodeTable[currentNode][SHORTEST_DISTANCE] + 1;
                unsigned int previousNode = nodeTable[nextNode][PREVIOUS_NODE];
                bool equal = (currentDistance == previousDistance);

                // check whether need to add label
                if (buildLabels && labelTable && path && previousDistance != INT_MAX) {
                    for (int i = 0; i < path->size(); i++) {
                        if (nextNode == (*path)[i]) {
                            // cout << currentNode << "->" << nextNode << endl;
                            // cout << "nextNode == (*path)[i] = " << nextNode << endl;
                            buildSegmentRoutingLabels(currentNode, nextNode, path, nodeTable, labelTable);
                            break; // Found our match, no need to continue
                        }
                    }
                }

                // Update node
                if (currentDistance < previousDistance || previousDistance == INT_MAX) {        
                    nodeTable[nextNode][PREVIOUS_NODE] = currentNode;
                    nodeTable[nextNode][SHORTEST_DISTANCE] = currentDistance;

                    if (nextNode != destination && previousDistance == INT_MAX) {
                        stack.push_back(nextNode);
                    }
                }
                else if (nextNode == destination && equal) {
                    if (matching(currentNode, nodeTable[nextNode][PREVIOUS_NODE], nodeTable)) {
                        nodeTable[nextNode][PREVIOUS_NODE] = currentNode;
                        nodeTable[nextNode][SHORTEST_DISTANCE] = currentDistance;
                    }
                }
            }
        }
    }
}

void TRA_switch:: shortestPaths(
    bool isCtrl,
    unsigned int source, 
    unsigned int destination,
    const map<unsigned int, map<unsigned int, pair<double, double>>>& networkTopology,
    int nodeTable[][3]) {
    traverseNetwork(source, destination, networkTopology, nodeTable, false, nullptr, nullptr);
    
    if (nodeTable[destination][SHORTEST_DISTANCE] == INT_MAX) return;
    if (isCtrl) routingTableInitiate(destination, source, nodeTable);
}

void TRA_switch:: routingTableInitiate(unsigned int destination, unsigned int source, int nodeTable[][3]) {
    unsigned int current = destination;
    vector<int> path;
    while (current != -1) 
    {
        path.push_back(current);
        current = nodeTable[current][PREVIOUS_NODE];
    }

    // Mapping by forward and backward
    int endIndex = path.size() - 1;
    for (int i = endIndex; i > 0; i--) {
        setMap(path[i], destination, path[i-1]);
    }
}

int main()
{  
    // header::header_generator::print(); // print all registered headers
    // payload::payload_generator::print(); // print all registered payloads
    // packet::packet_generator::print(); // print all registered packets
    // node::node_generator::print(); // print all registered nodes
    // event::event_generator::print(); // print all registered events
    // link::link_generator::print(); // print all registered links


    // #Switches #Links #FlowPairs #Labels BroadcastPeriod SimulateTime
    unsigned int num_switch, num_links, num_pairs, max_label, period, simulate_time;
    cin >> num_switch >> num_links >> num_pairs >> max_label >> period >> simulate_time;

    // read the input and generate switch nodes
    vector<node*> switches;  // Store all switches here
    for (unsigned int switch_i = 0; switch_i < num_switch; switch_i++) {
        node* new_switch = node::node_generator::generate("TRA_switch", switch_i, num_switch, num_links * 2);
        node::id_to_node(switch_i)->setNumOfLabel(max_label);
        if (new_switch != nullptr) switches.push_back(new_switch);
    }

    // LinkID Node1 Node2 Capacity
    unsigned int link_id, node1, node2;
    double capacity;
    for (unsigned int switch_i = 0; switch_i < num_links; switch_i++){
        cin >> link_id >> node1 >> node2 >> capacity;
        node::id_to_node(node1)->add_phy_neighbor(node2, "simple_link", {{"capacity", capacity}});
        node::id_to_node(node2)->add_phy_neighbor(node1, "simple_link", {{"capacity", capacity}});
    }

    // FlowID Src Dst FlowSize ArriveTime
    vector<tuple<unsigned int, unsigned int, unsigned int, unsigned int, unsigned int>> flows;
    unsigned int flow_id, src, dst, flowSize, time;

    for (unsigned int flow_i = 0; flow_i < num_pairs; flow_i++) {
        cin >> flow_id >> src >> dst >> flowSize >> time;
        flows.emplace_back(flow_id, src, dst, flowSize, time);
    }
    unsigned int t = 0;
    for(unsigned int t = 0; t <= simulate_time; t += period)
    {
        for (unsigned int switch_i = 0; switch_i < num_switch; switch_i++) 
            TRA_ctrl_packet_event(switch_i, t);
    }

    for (const auto& flow : flows) {
        data_packet_event(get<1>(flow), get<2>(flow), get<3>(flow), get<4>(flow));
    }


    // start simulation!!
    event::start_simulate(simulate_time);

    for (unsigned int i = 0; i < switches.size(); i++) {
        TRA_switch* tra_switch = dynamic_cast<TRA_switch*>(switches[i]);
        if (tra_switch) {
            cout << i << endl;
            map<unsigned int, unsigned int> rt = tra_switch->getRoutingTable();
            for (const auto& entry : rt) {
                cout << entry.first << " " << entry.second << endl;
            }
        }
    }

    // event::flush_events() ;
    // cout << packet::getLivePacketNum() << endl;
    return 0;
}
