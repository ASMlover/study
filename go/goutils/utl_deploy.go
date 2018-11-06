package main

import (
	"encoding/json"
	"flag"
	"fmt"
	"os"
	"path"
	"path/filepath"
)

type NewPorter struct {
	port int
}

func (np *NewPorter) __gen_port() int {
	r := np.port
	np.port += 1
	return r
}

func (np *NewPorter) __gen_conf() map[string]int {
	return map[string]int{
		"port": np.__gen_port(),
	}
}

func __write_conf(fullpath string, conf []byte) (int, error) {
	os.MkdirAll(path.Dir(fullpath), os.ModePerm)
	fw, err := os.Create(fullpath)
	if err != nil {
		return 0, err
	}
	defer fw.Close()

	return fw.Write(conf)
}

func __generate_conf(cur_dirpath string, ip string, port int) {
	conf_fields := []string{
		"dbmanager0", "gamemanager1", "gamemanager2", "gamemanager3", "game0", "game1",
	}

	h56conf := make(map[string]interface{})
	np := NewPorter{port}
	h56conf["gamemanager"] = map[string]int{
		"desired_games": 2,
		"desired_gates": 1,
	}
	for _, key := range conf_fields {
		h56conf[key] = np.__gen_conf()
	}
	gateport := np.__gen_port()
	h56conf["gate0"] = map[string]interface{}{
		"publicip":   ip,
		"publicport": gateport,
		"port":       gateport,
		"innerport":  gateport,
	}

	h56conf_json, err := json.MarshalIndent(h56conf, "", "\t")
	if err != nil {
		return
	}

	conf_dirpath := fmt.Sprintf("%s/local.conf", cur_dirpath)
	__write_conf(conf_dirpath, h56conf_json)
}

func main() {
	usage := func() {
		fmt.Fprintf(os.Stderr, "Usage: ./deploy [-addr=<ipaddr>] [-port=<startport>]\n")
		flag.PrintDefaults()
	}

	addr := flag.String("addr", "127.0.0.1", "public ip address, must be real ip address")
	port := flag.Int("port", 4000, "start port for configuration")
	flag.Usage = usage
	flag.Parse()

	if flag.NFlag() < 2 {
		usage()
	} else {
		cur_dirpath, err := filepath.Abs(filepath.Dir(os.Args[0]))
		if err != nil {
			fmt.Fprintf(os.Stderr, "deploy exec failed !!!\n")
			return
		}
		__generate_conf(cur_dirpath, *addr, *port)
	}
}
