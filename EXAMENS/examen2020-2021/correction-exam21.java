//Q6

public Root(){
    ipSon1==null;
    ipSon2==null;
    myIp=InetAddress.getLocalHost().getHostAddress();
}

//Q7

public String getIp(Socket s){
    return s.getInetAddress().getHostAddress();
}

//Q8
public boolean setSon(Socket s){
    if(ipSon1==null){
        ipSon1=getIp(s);
        return true;
    } else if(ipSon2==null){
        ipSon2=getIp(s);
        return true;
    }else{
        return false;
    }
}

//Q9

public void transMess(String mess){
    try{
        DatagramSocket dso=new DatagramSocket();
        byte[]data;
        String s="T "+mess;
        data=s.getBytes();
        if(ipSon1!=null){
            InetSocketAddress ia=new InetSocketAddress(ipSon1,1234);
            DatagramPacket paquet=
                new DatagramPacket(data,data.length,ia);
            dso.send(paquet);
        }
        if(ipSon2!=null){
            InetSocketAddress ia=new InetSocketAddress(ipSon2,1234);
            DatagramPacket paquet=
                new DatagramPacket(data,data.length,ia);
            dso.send(paquet);
        }
    } catch(Exception e){
        e.printStackTrace();
    }
}

//Q10

public void treatLeaf(){
    if(ipSon1==null || ipSon2==null){
        DatagramSocket dso=new DatagramSocket();
        byte[]data;
        String s="L "+myIp;
        data=s.getBytes();
        InetSocketAddress ia=new InetSocketAddress("233.222.222.1",4242);
        DatagramPacket paquet=new 
            DatagramPacket(data,data.length,ia);
        dso.send(paquet);
    } else{
        DatagramSocket dso=new DatagramSocket();
        byte[]data;
        String s="FREE";
        InetSocketAddress ia=new InetSocketAddress(ipSon1,1234);
        DatagramPacket paquet=new DatagramPacket(data,data.length,ia);
        dso.send(paquet);
        ia=new InetSocketAddress(ipSon2,1234);
        paquet=new DatagramPacket(data,data.length,ia);
        dso.send(paquet);
     
    }
}

//Q11

public void treatTCPmess(Socket s,String mess){
    if(mess.equals("LEAF")){
        Thread.sleep(1000);
        treatLeaf();
    }else if(mess.equals("CONNECT\n")){
        PrintWriter pw=new PrintWriter(new  OutputStreamWriter(s.getOutputStream()));
        if(setSon(s)){
            pw.print("OKSON\n");
            pw.flush();
        }else{
            pw.print("FULL\n");
            pw.flush();
        }
    }else{
        String m=mess.substring(2,mess.length());
        transMess(m);
    }
}    

//Q12.
public void run(){
    BufferedReader br=new BufferedReader(
                   new InputStreamReader(sock.getInputStream()));
    String mess=br.readLine();
    ro.treatTCPmess(mess+"\n");
    br.close();
    sock.close();
}

//Q13

public static void main(String[] argv){
    Root ro=new Root():
    try{
            ServerSocket server=new ServerSocket(5678);
            while(true){
                Socket socket=server.accept();
                TCPService serv=new TCPService(ro,socket);
                Thread t=new Thread(serv);
                t.start();
            }   
        }
        catch(Exception e){
            System.out.println(e);
            e.printStackTrace();
        }
}

 
//Q14

/*Le noeud racine pourrait leur répondre aux deux qu'ils sont fils1 à cause de problème de concurrence (si par exemple la méthode setSon est appelée en même temps). Une solution est de mettre toutes les méthodes accedant à ipSon1 et ipSon2 en synchronized.*/
