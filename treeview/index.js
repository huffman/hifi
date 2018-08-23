console.log("HI");


class Tree {
    constructor(rootEl) {
        this.rootEl = rootEl;
        this.items = {};
    }
    move(childID, parentID) {
        let item = this.items[childID];
        if (item.parentID !== null) {
            let oldParentItem = this.items[item.parentID];
            oldParentItem.children.splice(oldParentItem.children.indexOf(childID), 1);
        }
        let parentItem = this.items[parentID];
        parentItem.children.push(childID);
        parentItem.elChildren.append(item.el);

    }
    add(id, name, parentID) {
        let tree = this;

        if (id in this.items) {
            throw "ID already in items: " + parentID;
        }

        // Create element
        let elRow = document.createElement("div");
        elRow.className = "row";
        elRow.setAttribute('draggable', true);
        elRow.ondragstart = function(ev) {
            ev.stopPropagation();
            //ev.dataTransfer.dropEffect = "move";
            console.log("Starting to drag: " + id, ev);
            ev.dataTransfer.setData('text/plain', '' + id);
            return true;
        }
        elRow.ondragenter = function(ev) {
            ev.stopPropagation();

            console.log("Drag enter: " + id, ev);
            if (ev.dataTransfer.getData('text/plain') !== id) {
                ev.preventDefault();
                this.className = "row drop-target";
            }
        }
        elRow.ondragleave = function(ev) {
            console.log("Drag exit: " + id, ev);
            ev.stopPropagation();
            ev.preventDefault();
            this.className = "row";
            return true;
        }
        elRow.ondragover = function(ev) {
            ev.stopPropagation();
            ev.preventDefault();
            return true;
        }
        elRow.ondrop = function(ev) {
            ev.stopPropagation();
            ev.preventDefault();
            let newChildID = ev.dataTransfer.getData('text/plain')
            tree.move(newChildID, id);
            this.className = "row";
            console.log("Dropped on: " + id, newChildID, ev);
        }

        let elContent = document.createElement("div");
        elContent.className = "content";
        elContent.innerHTML = name;

        let elChildren = document.createElement("div");
        elChildren.className = "children";

        elRow.append(elContent);
        elRow.append(elChildren);

        // TODO(huffman) handle case where item has parent id that is unknown,
        // TODO(huffman) handle case where items are parented to this item
        this.items[id] = {
            name: name,
            parentID: parentID,
            el: elRow,
            elChildren: elChildren,
            children: [],
        }

        if (parentID === null) {
            this.rootEl.append(elRow);
        } else {
            let parentItem = this.items[parentID];
            if (parentItem === undefined) {
                throw "Unknown parent ID: " + parentID;
            }
            parentItem.children.push(id);
            parentItem.elChildren.append(elRow);
        }
    }
}

var tree = new Tree(document.getElementById('tree'));


let _id = 0;
function getid() {
    return _id++;
}

let cnts = [0, 0, 0, 3, 1];
function recursiveAdd(parentID) {
    let childCnt = cnts[Math.floor(Math.random() * cnts.length)];
    for (let i = 0; i < childCnt; ++i) {
        let id = getid();
        tree.add(id, id + "", parentID);
        recursiveAdd(id);
    }
}

tree.add(getid(), "0", null);
tree.add(getid(), "1", 0);
tree.add(getid(), "2", 0);
tree.add(getid(), "3", null);
for (let i = 4; i < 20; i++) {
    let topid = getid();
    tree.add(topid, topid + "", 3);
    for (let j = 0, cnt = Math.floor(Math.random() * 10); j < cnt; ++j) {
        let id = getid();
        tree.add(id, id + "", topid);
    }
}

recursiveAdd(0);
