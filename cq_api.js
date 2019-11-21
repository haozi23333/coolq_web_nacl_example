class CQServer {
    constructor() {
        this.load_nacl_plugin();
    }

    load_nacl_plugin() {
        this.cq_el = document.createElement('embed');
        this.cq_el.setAttribute('name', 'moto_native_cl');
        this.cq_el.setAttribute('id', 'moto_native_cl');
        this.cq_el.setAttribute('width', 0);
        this.cq_el.setAttribute('height', 0);
        this.cq_el.setAttribute('path', 'pnacl');
        this.cq_el.setAttribute('src', './pnacl/Release/web_app.nmf');
        this.cq_el.setAttribute('type', 'application/x-pnacl');
        this.cq_el.addEventListener('message', ({data}) => this.on_message(data))
        document.body.append(this.cq_el)
    }

    on_message(data) {
        if (data === "SYSTEM:SUCCESS") {
            this.heart_rate_service(true)
        }
        if (data.slice(0, 3) === "CQ_") {
            this.on_qq_message(data.slice(3, -1));
        }
    }

    send(raw_string) {
        this.cq_el.postMessage(raw_string);
    }

    heart_rate_service(immediately = false) {
        setTimeout(() => {
            console.log('心跳')
            this.send("say_hello");
            this.heart_rate_service();
        }, immediately ? 1 : 300000)
    }

    on_qq_message(raw_message) {
        console.log(`QQ 消息 -> ${raw_message}`)
    }
}


document.body.onload = () => {
    window.cq = new CQServer()
}