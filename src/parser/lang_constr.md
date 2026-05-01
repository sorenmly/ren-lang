
## AST RULES for further anotation !1!11!

### VarDeclNode { name, value }
var x = 10;

### SceneDeclNode { name, body: std::vector<Node> }
scene idk { ... }

### AnimateDeclNode { body: std::vector<TransitionNode>}
animate { zoom from 1.0 to 0.5 easing expo}

### ShaderDeclNode { type, body: std::string}
shader frag { ... }
