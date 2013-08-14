<html>
  <head>
    <title></title>
    <body>
      <form action="/login" method="post">
        <input type="checkbox" name="interest" value="football">足球
        <input type="checkbox" name="interest" value="basketball">篮球
        <input type="checkbox" name="interest" value="tennis">网球<br/>
        UserName: <input type="text" name="username">
        Password: <input type="password" name="password">
        <input type="hidden" name="token" value="{{.}}">
        <input type="submit" value="Login">
      </form>
    </body>
  </head>
</html>
