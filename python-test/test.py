import tornado.web 
import tornadoredis

CONNECTION_POOL = tornadoredis.ConnectionPool(max_connections=500,wait_for_available=True)
c = tornadoredis.Client(connection_pool=CONNECTION_POOL)

class test(tornado.web.RequestHandler):
	@tornado.web.asynchronous
	@tornado.gen.coroutine 
	def get(self):
		data = yield tornado.gen.Task(c.set, 'test', '123')
		print(data)
		self.set_status(200)
		self.write('yangmeng')
		self.finish()
		

if __name__ == '__main__':
	app = tornado.web.Application(handlers=[
		(r'/test', test)])
	server = tornado.httpserver.HTTPServer(app)
	server.bind(8002, reuse_port = True)
	server.start(1)
	tornado.ioloop.IOLoop.instance().start()

